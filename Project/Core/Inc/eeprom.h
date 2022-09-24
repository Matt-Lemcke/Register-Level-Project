/*
 * eeprom.h
 *
 *  Created on: Sep. 20, 2022
 *      Author: lemck
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "i2c.h"

// Device I2C slave address
#define GET_ADDR(a0, a1, a2) ((5<<4)|(a2<<2)|(a1<<1)|(a0<<0))

// Memory addresses
#define ADDR_BOTTOM 	0x0000
#define ADDR_TOP		0x1FFF

typedef enum{
	EEPROM_OK,
	EEPROM_NO_DEVICE,
	EEPROM_ERR
}EEPROM_Status_e;

typedef struct{
	uint8_t address;
	I2C_Handler_t *i2c;
}EEPROM_t;

EEPROM_Status_e EEPROM_Init(EEPROM_t *handler);


#endif /* INC_EEPROM_H_ */
