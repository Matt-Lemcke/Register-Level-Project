/*
 * i2c.c
 *
 *  Created on: Sep. 13, 2022
 *      Author: lemck
 */
#include "i2c.h"
#include "clocks.h"

#define SLAVE_WRITE(addr)	((addr<<1) & ~(1<<0))
#define SLAVE_READ(addr)	((addr<<1) |  (1<<0))

// Private Function definition
void I2C_Reset(I2C_Handler_t *handler);
void I2C_Clear_Buffer(I2C_Handler_t *handler);
void I2C_Start(I2C_Handler_t *handler);
void I2C_Stop(I2C_Handler_t *handler);
I2C_Status_e I2C_Write_Addr(I2C_Handler_t *handler, uint8_t addr);
I2C_Status_e I2C_Write_Bytes(I2C_Handler_t *handler, uint8_t size);
I2C_Status_e I2C_Read_Bytes(I2C_Handler_t *handler, uint8_t size);

/*	----------------------
*	Public Functions
*	----------------------
*/

I2C_Status_e I2C_Init(I2C_Handler_t *handler){

	GPIO_Handle_t gpio_handler;

	// Check clock and i2c bus speeds are compatible in standard mode
	if(APB1_CLK_MHz<2 || APB1_CLK_MHz>50){
		return I2C_CLK_ERR;
	}
	if(handler->speed > 100 || handler->speed <= 0){
		return I2C_ERR;
	}

	// Configure peripheral clocks and assign GPIO register pointer
	if(handler->I2C == I2C1){
		// GPIO pins PB8 and PB9
		handler->scl_pin = 8;
		handler->sda_pin = 9;

		ENABLE_GPIOB;
		gpio_handler.port = GPIOB;

		RCC->APB1ENR |= (1<<21);
	}
	else if(handler->I2C == I2C2){
		// GPIO pins PF0 and PF1
		handler->scl_pin = 1;
		handler->sda_pin = 0;

		ENABLE_GPIOF;
		gpio_handler.port = GPIOF;

		RCC->APB1ENR |= (1<<22);
	}
	else if(handler->I2C == I2C3){
		// GPIO pins PH7 and PH8
		handler->scl_pin = 7;
		handler->sda_pin = 8;

		ENABLE_GPIOH;
		gpio_handler.port = GPIOH;

		RCC->APB1ENR |= (1<<23);
	}
	else{
		return I2C_ERR;
	}

	// Configure GPIO pins for i2c alternate function
	gpio_handler.mode = SET_MODE(handler->sda_pin, MODE_AF) | SET_MODE(handler->scl_pin, MODE_AF);
	gpio_handler.alt = SET_AF(handler->sda_pin, AF4) | SET_AF(handler->scl_pin, AF4);
	gpio_handler.out = SET_OUT(handler->sda_pin, OUT_DRAIN) | SET_OUT(handler->scl_pin, OUT_DRAIN);
	gpio_handler.speed = SET_SPEED(handler->sda_pin, SPEED_VHI) | SET_SPEED(handler->scl_pin, SPEED_VHI);
	gpio_handler.res = SET_RES(handler->sda_pin, RES_NONE) | SET_RES(handler->scl_pin, RES_NONE);

	// Initialize GPIO
	GPIO_Init(&gpio_handler);
	handler->gpio = &gpio_handler;

	// Clear I2C bus
	I2C_Reset(handler);
	I2C_Clear_Buffer(handler);

	// Set bus clock frequency
	handler->I2C->CR2 |= APB1_CLK_MHz;

	// Set CCR
	// CCR = F_PCLK / (2*F_I2C) for 50% duty cycle
	handler->I2C->CCR |= APB1_CLK_MHz*1000/(2*handler->speed);

	// Set TRISE
	// TRISE = F_PLCK * 1000ns + 1 for 50% duty cycle
	handler->I2C->TRISE = APB1_CLK_MHz +1;

	// Enable I2C
	handler->I2C->CR1 |= (1<<0);

	return I2C_OK;
}

I2C_Status_e I2C_Write(I2C_Handler_t *handler, uint8_t slave_addr, uint8_t size, uint8_t send_stop){

	// Send START and device address in write mode
	if(I2C_Write_Addr(handler, SLAVE_WRITE(slave_addr)) != I2C_OK){
		return I2C_ERR;
	}
	I2C_Write_Bytes(handler, size);			// Write data from buffer
	I2C_Clear_Buffer(handler);			// Clear data buffer
	if(send_stop){
		I2C_Stop(handler);			// Send STOP
	}
	return I2C_OK;
}

I2C_Status_e I2C_Read(I2C_Handler_t *handler, uint8_t slave_addr, uint8_t size){
	
	I2C_Clear_Buffer(handler);			// Clear data buffer
	
	// Send START and device address in read mode
	if(I2C_Write_Addr(handler, SLAVE_READ(slave_addr)) != I2C_OK){
		return I2C_ERR;	
	}	
	I2C_Read_Bytes(handler, size);			// Read data into buffer
	I2C_Stop(handler);				// Send STOP
	return I2C_OK;
}

I2C_Status_e I2C_Test_Device(I2C_Handler_t *handler, uint8_t slave_addr){
	
	// Send START and device address in write mode
	if(I2C_Write_Addr(handler, SLAVE_WRITE(slave_addr)) != I2C_OK){
		return I2C_ERR;
	}
	return I2C_OK;
}

/*	----------------------
*	Private Functions
*	----------------------
*/

void I2C_Reset(I2C_Handler_t *handler){
	// Toggle reset bit
	handler->I2C->CR1 |= (1<<15);
	Delay(10);
	handler->I2C->CR1 &= ~(1<<15);
}

void I2C_Clear_Buffer(I2C_Handler_t *handler){
	
	// Set all buffer values to 0
	for(int i = BUFFER_SIZE+2; i>0; i--){
		handler->buffer[i-1] = 0;
	}
}

void I2C_Start(I2C_Handler_t *handler){
	handler->I2C->CR1 |= (1<<10);			// Enable ACK
	handler->I2C->CR1 |= (1<<8);			// Send START
	while(!(handler->I2C->SR1 & (1<<0)));		// Wait for START to send
}

void I2C_Stop(I2C_Handler_t *handler){
	handler->I2C->CR1 |= (1<<9);			// Send STOP
}

I2C_Status_e I2C_Write_Addr(I2C_Handler_t *handler, uint8_t addr){
	uint8_t status = I2C_DEV_BUSY;
	uint8_t count = MAX_ATTEMPTS;

	// Attempt to connect to device when free
	while(count && status == I2C_DEV_BUSY){
		I2C_Start(handler);					// Send START				
		uint16_t temp = handler->I2C->SR1 | handler->I2C->SR2;	// Read registers to clear SB flag
		handler->I2C->DR = addr;				// Write address of slave device

		Delay(5);
		if(handler->I2C->SR1 & (1<<1)){				// Check if ACK received
			status = I2C_OK;
		}
		count--;
	}
	uint16_t temp = handler->I2C->SR1 | handler->I2C->SR2;		// Read registers to clear SB flag
	return status;
}

I2C_Status_e I2C_Write_Bytes(I2C_Handler_t *handler, uint8_t size){
	uint8_t temp = handler->I2C->SR1 | handler->I2C->SR2;			// Read to reset ADDR flag
	for(int i = 0; i<size; i++){
		while(!(handler->I2C->SR1 & (1<<7)));				// Wait for TxE flag to set
		handler->I2C->DR = (uint16_t)(handler->buffer[i]) & 0x00FF;	// Write data from buffer to data register (keep bits 8:15 as 0)
	}
	while(!(handler->I2C->SR1 & (1<<7))&&!(handler->I2C->SR1 & (1<<2)));	// Wait for TxE or BTF flags to set
	return I2C_OK;
}

I2C_Status_e I2C_Read_Bytes(I2C_Handler_t *handler, uint8_t size){
	// Read 1 Byte
	if(size < 2){
		if(!size){
			// 0 data size given
			return I2C_ERR;
		}
		handler->I2C->CR1 &= ~(1<<10);						// Clear ACK bit
		uint8_t temp = handler->I2C->SR1 | handler->I2C->SR2;			// Read to reset ADDR flag
		I2C_Stop(handler);							// Send STOP
		while(!(handler->I2C->SR1 & (1<<6)));					// Wait for RxNE flag
		handler->buffer[0] = (uint8_t)(handler->I2C->DR & 0x00FF);		// Read data from register into buffer (ignore reserved bits 8:15)
	}
	
	// Read 2+ Bytes
	else{
		uint8_t temp = handler->I2C->SR1 | handler->I2C->SR2;			// Read to reset ADDR flag
		
		// Read all but last two bytes
		for(int i = 0; i<size-2; i++){
			while(!(handler->I2C->SR1 & (1<<6)));				// Wait for RxNE flag
			handler->buffer[i] = (uint8_t)(handler->I2C->DR & 0x00FF);	// Read data from register into buffer (ignore reserved bits 8:15)
			handler->I2C->CR1 |= (1<<6);					// Set ACK bit
		}
		// Read second last Byte
		while(!(handler->I2C->SR1 & (1<<6)));					// Wait for RxNE flag
		handler->buffer[size-2] = (uint8_t)(handler->I2C->DR & 0x00FF);		// Read data from register into buffer (ignore reserved bits 8:15)
		handler->I2C->CR1 &= ~(1<<10);						// Clear ACK bit
		I2C_Stop(handler);							// Send STOP

		// Read last Byte
		while(!(handler->I2C->SR1 & (1<<6)));					// Wait for RxNE flag
		handler->buffer[size-1] = (uint8_t)(handler->I2C->DR & 0x00FF);		// Read data from register into buffer (ignore reserved bits 8:15)
	}
	return I2C_OK;
}
