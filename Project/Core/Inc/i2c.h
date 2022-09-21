/*
 * i2c.h
 *
 *  Created on: Sep. 13, 2022
 *      Author: lemck
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include <stdlib.h>
#include "stm32f429xx.h"
#include "gpio.h"

#define BUFFER_SIZE 32
#define MAX_ATTEMPTS 20

#define NO_STOP	0
#define STOP 	1

typedef enum {
	I2C_OK = 1,
	I2C_ERR,
	I2C_CLK_ERR,
	I2C_DEV_BUSY,
	I2C_NACK
}I2C_Status_e;

// Change later to adjust for padding
typedef struct {
	uint16_t speed; // In kHz // 2 bytes
	uint8_t scl_pin : 4;
	uint8_t sda_pin	: 4;

	I2C_TypeDef *I2C; // 4 bytes

	uint8_t buffer[BUFFER_SIZE];	// 32 bytes

	GPIO_Handle_t *gpio;

}I2C_Handler_t;


I2C_Status_e I2C_Init(I2C_Handler_t *handler);
I2C_Status_e I2C_Test_Device(I2C_Handler_t *handler, uint8_t slave_addr);
I2C_Status_e I2C_Write(I2C_Handler_t *handler, uint8_t slave_addr, uint8_t size, uint8_t send_stop);
I2C_Status_e I2C_Read(I2C_Handler_t *handler, uint8_t slave_addr, uint8_t size);

#endif /* INC_I2C_H_ */
