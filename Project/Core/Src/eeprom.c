/*
 * eeprom.c
 *
 *  Created on: Sep. 20, 2022
 *      Author: lemck
 */


#include "eeprom.h"

#define PAGE_SIZE	32

EEPROM_Status_e EEPROM_Random_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr);
EEPROM_Status_e EEPROM_Current_Read(EEPROM_t *handler, uint8_t *data);
EEPROM_Status_e EEPROM_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr);
EEPROM_Status_e EEPROM_Multi_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size);
EEPROM_Status_e EEPROM_Sequential_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size);

EEPROM_Status_e EEPROM_Init(EEPROM_t *handler){

	// Check device connection
	if(I2C_Test_Device(handler->i2c, handler->address)!=I2C_OK){
		return EEPROM_NO_DEVICE;
	}

	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr){
	if(mem_addr > ADDR_TOP){
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
	handler->i2c->buffer[2] = *data;
	if(I2C_Write(handler->i2c, handler->address, 3, STOP)!=I2C_OK){
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Multi_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size){
	uint8_t buffer_index = 0;
	if(mem_addr > ADDR_TOP || size > BUFFER_SIZE){
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
	for(int i = 0; i < size; i++){
		if(mem_addr % PAGE_SIZE || i==0){
			handler->i2c->buffer[buffer_index+2] = data[i];
			mem_addr++;
			buffer_index++;
		}
		else{
			if(I2C_Write(handler->i2c, handler->address, buffer_index+2, STOP)!=I2C_OK){
				return EEPROM_ERR;
			}
			buffer_index = 0;
			handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
			handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
			handler->i2c->buffer[buffer_index+2] = data[i];
			mem_addr++;
		}
	}
	if(I2C_Write(handler->i2c, handler->address, buffer_index+2, STOP)!=I2C_OK){
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Current_Read(EEPROM_t *handler, uint8_t *data){
	if(I2C_Read(handler->i2c, handler->address, 1)!=I2C_OK){
		return EEPROM_ERR;
	}
	*data = handler->i2c->buffer[0];
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Random_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr){
	if(mem_addr > ADDR_TOP){
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
	if(I2C_Write(handler->i2c, handler->address, 2, NO_STOP)!=I2C_OK){
		return EEPROM_ERR;
	}
	if(I2C_Read(handler->i2c, handler->address, 1)!=I2C_OK){
		return EEPROM_ERR;
	}
	*data = handler->i2c->buffer[0];
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Sequential_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size){
	if(mem_addr > ADDR_TOP || size > BUFFER_SIZE){
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
	if(I2C_Write(handler->i2c, handler->address, 2, NO_STOP)!=I2C_OK){
		return EEPROM_ERR;
	}
	if(I2C_Read(handler->i2c, handler->address, size)!=I2C_OK){
		return EEPROM_ERR;
	}
	while(size>0){
		data[size-1] = handler->i2c->buffer[size-1];
		size--;
	}
	return EEPROM_OK;
}
