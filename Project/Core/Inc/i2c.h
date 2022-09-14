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

typedef enum {
	I2C_OK,
	I2C_ERR,
	I2C_CLK_ERR
}I2C_Status_e;

typedef enum {
	I2C_1 = 0,
	I2C_2 = 1,
	I2C_3 = 2,
	I2C_Port_Count = 3
}I2C_Port_e;

typedef struct {
	I2C_Port_e port;
	uint16_t speed;

	I2C_TypeDef *I2C;

}I2C_Handler_t;

I2C_Status_e I2C_Init(I2C_Handler_t *handler);

#endif /* INC_I2C_H_ */
