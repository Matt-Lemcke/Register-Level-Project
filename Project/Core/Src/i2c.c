/*
 * i2c.c
 *
 *  Created on: Sep. 13, 2022
 *      Author: lemck
 */
#include "i2c.h"
#include "clocks.h"

// Bit mask to enable peripheral clocks
#define APB1_I2C(__port__) (1<<(21+__port__))
#define AHB1_I2C(__port__) (__port__ > 0) ? (1<<(6+__port__)) : (1<<1)

// Bit masks for GPIO settings
#define GPIO_AF(__pin__) (2<<(__pin__<<1))
#define GPIO_OPENDRAIN(__pin__) (1<<__pin__)
#define GPIO_VHIGH_SPEED(__pin__) (3<<(__pin__<<1))
#define GPIO_PULLUP(__pin__) (1<<(__pin__<<1))
#define GPIO_AF_I2C(__pin__) (__pin__ >= 8) ? (4<<(__pin__ + __pin__ -16)) : (4<<(__pin__ + __pin__ ))

// Private Functions
void I2C_Reset(I2C_Handler_t *handler);


I2C_Status_e I2C_Init(I2C_Handler_t *handler){

	if(handler->port >= I2C_Port_Count){
		return I2C_ERR;
	}
	if(APB1_CLK_MHz<2 || APB1_CLK_MHz>50){
		return I2C_CLK_ERR;
	}

	// Configure peripheral clock bus
	RCC->APB1ENR |= APB1_I2C(handler->port);
	RCC->AHB1ENR |= AHB1_I2C(handler->port);

	// Configure GPIO and assign I2C register pointer
	switch(handler->port){
		case I2C_1:
			handler->I2C = I2C1;
			GPIOB->MODER |= GPIO_AF(8) | GPIO_AF(9);
			GPIOB->OTYPER |= GPIO_OPENDRAIN(8) | GPIO_OPENDRAIN(9);
			GPIOB->OSPEEDR |= GPIO_VHIGH_SPEED(8) | GPIO_VHIGH_SPEED(9);
			GPIOB->PUPDR |= GPIO_PULLUP(8) | GPIO_PULLUP(9);
			GPIOB->AFR[1] |= GPIO_AF_I2C(8) | GPIO_AF_I2C(9);
			break;
		case I2C_2:
			handler->I2C = I2C2;
			GPIOF->MODER |= GPIO_AF(0) | GPIO_AF(1);
			GPIOF->OTYPER |= GPIO_OPENDRAIN(0) | GPIO_OPENDRAIN(1);
			GPIOF->OSPEEDR |= GPIO_VHIGH_SPEED(0) | GPIO_VHIGH_SPEED(1);
			GPIOF->PUPDR |= GPIO_PULLUP(0) | GPIO_PULLUP(1);
			GPIOF->AFR[0] |= GPIO_AF_I2C(0) | GPIO_AF_I2C(1);
			break;
		case I2C_3:
			handler->I2C = I2C3;
			GPIOH->MODER |= GPIO_AF(7) | GPIO_AF(8);
			GPIOH->OTYPER |= GPIO_OPENDRAIN(7) | GPIO_OPENDRAIN(8);
			GPIOH->OSPEEDR |= GPIO_VHIGH_SPEED(7) | GPIO_VHIGH_SPEED(8);
			GPIOH->PUPDR |= GPIO_PULLUP(7) | GPIO_PULLUP(8);
			GPIOH->AFR[0] |= GPIO_AF_I2C(7);
			GPIOH->AFR[1] |= GPIO_AF_I2C(8);
			break;
		default:
			break;
	}

	I2C_Reset(handler);

	// Set bus clock frequency
	handler->I2C->CR2 |= APB1_CLK_MHz;
	//Set CCR
	// Set TRISE

	// Enable I2C
	handler->I2C->CR1 |= (1<<0);


	return I2C_OK;


}

void I2C_Reset(I2C_Handler_t *handler){
	handler->I2C->CR1 |= (1<<15);
	handler->I2C->CR1 &= ~(1<<15);
}

