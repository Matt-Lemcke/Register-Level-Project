/*
 * i2c.c
 *
 *  Created on: Sep. 13, 2022
 *      Author: lemck
 */
#include "i2c.h"
#include "clocks.h"

// Bit masks for GPIO settings
#define GPIO_AF(__pin__) (2<<(__pin__<<1))
#define GPIO_OUT(__pin__) (1<<(__pin__<<1))
#define GPIO_OPENDRAIN(__pin__) (1<<__pin__)
#define GPIO_VHIGH_SPEED(__pin__) (3<<(__pin__<<1))
#define GPIO_PULLUP(__pin__) (0<<(__pin__<<1))
#define GPIO_AF_I2C(__pin__) (4<<((__pin__)<<2))

// Private Functions
void I2C_Reset(I2C_Handler_t *handler);
void I2C_Start(I2C_Handler_t *handler);
void I2C_WriteAddr(I2C_Handler_t *handler, uint8_t addr);
void I2C_WriteData(I2C_Handler_t *handler, uint8_t data);

I2C_Status_e Workaround(I2C_Handler_t *handler);


I2C_Status_e I2C_Init(I2C_Handler_t *handler){

	if(APB1_CLK_MHz<2 || APB1_CLK_MHz>50){
		return I2C_CLK_ERR;
	}
	if(handler->speed > 100 || handler->speed <= 0){
		return I2C_ERR;
	}

	// Configure peripheral clocks and assign GPIO register pointer
	if(handler->I2C == I2C1){
		handler->GPIO = GPIOB;
		handler->scl_pin = 8;
		handler->sda_pin = 9;

		RCC->AHB1ENR |= (1<<1);
		RCC->APB1ENR |= (1<<21);
	}
	else if(handler->I2C == I2C2){
		handler->GPIO = GPIOF;
		handler->scl_pin = 1;
		handler->sda_pin = 0;

		RCC->AHB1ENR |= (1<<5);
		RCC->APB1ENR |= (1<<22);
	}
	else if(handler->I2C == I2C3){
		handler->GPIO = GPIOH;
		handler->scl_pin = 7;
		handler->sda_pin = 8;

		RCC->AHB1ENR |= (1<<7);
		RCC->APB1ENR |= (1<<23);
	}
	else{
		return I2C_ERR;
	}

	// Configure GPIO pins
	if(handler->scl_pin > 7){
		handler->GPIO->AFR[1] |= GPIO_AF_I2C(handler->scl_pin - 8);
	}
	else{
		handler->GPIO->AFR[0] |= GPIO_AF_I2C(handler->scl_pin);
	}
	if(handler->sda_pin > 7){
		handler->GPIO->AFR[1] |= GPIO_AF_I2C(handler->sda_pin - 8);
	}
	else{
		handler->GPIO->AFR[0] |= GPIO_AF_I2C(handler->sda_pin);
	}

	handler->GPIO->MODER |= GPIO_AF(handler->sda_pin) | GPIO_AF(handler->scl_pin);
	handler->GPIO->OTYPER |= GPIO_OPENDRAIN(handler->sda_pin) | GPIO_OPENDRAIN(handler->scl_pin);
	handler->GPIO->OSPEEDR |= GPIO_VHIGH_SPEED(handler->sda_pin) | GPIO_VHIGH_SPEED(handler->scl_pin);
	handler->GPIO->PUPDR |= GPIO_PULLUP(handler->sda_pin) | GPIO_PULLUP(handler->scl_pin);

	I2C_Reset(handler);

	// Set bus clock frequency
	handler->I2C->CR2 |= APB1_CLK_MHz;

//	// Enable error inturrupts
//	handler->I2C->CR2 |= (1<<8);
//	NVIC_SetPriority(I2C1_ER_IRQn, 1);
//	NVIC_EnableIRQ(I2C1_ER_IRQn);

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

void I2C_Reset(I2C_Handler_t *handler){
	handler->I2C->CR1 |= (1<<15);	// Toggle reset bit
	Delay(10);
	handler->I2C->CR1 &= ~(1<<15);
}

void I2C_Start(I2C_Handler_t *handler){
	handler->I2C->CR1 |= (1<<10);		// Enable ACK
	handler->I2C->CR1 |= (1<<8);		// Send START

	// Read registers to clear flag
	uint8_t sr1 = handler->I2C->SR1;
	uint8_t sr2 = handler->I2C->SR2;
	while(!(handler->I2C->SR1 & (1<<0)));	// Wait for START to send
}

void I2C_WriteAddr(I2C_Handler_t *handler, uint8_t addr){
	handler->I2C->DR = addr;								// Write address of slave device
	while(!(handler->I2C->SR1 & (1<<1)));					// Wait to receive ACK
	uint8_t temp = handler->I2C->SR1 | handler->I2C->SR2;	// Read to reset ADDR flag
}

I2C_Status_e I2C_Test_Device(I2C_Handler_t *handler, uint8_t addr){
	I2C_Start(handler);							// Send START
	I2C_WriteAddr(handler, (addr<<1)|(0<<0));	// Send Address with write command
	return I2C_OK;
}

