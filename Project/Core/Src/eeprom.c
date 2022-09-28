/*
 * eeprom.c
 *
 *  Created on: Sep. 20, 2022
 *      Author: lemck
 */


#include "eeprom.h"

#define MEMORY_INITIALIZED

#define PAGE_SIZE	64
#define DATA_PAGES	126

// Memory addresses
#define POINTER_POINTER_ADDR	0x0000
#define POINTER_BASE_ADDR		0x0001
#define DATA_BASE_ADDR			0x0080
#define DATA_TOP_ADDR			0x1FFF

#define ALIGN_ADDR(addr)		(addr=(addr & 0x00FF)<<6)

uint16_t current_page_addr;		// Current page address
uint16_t page_pointer_addr;		// Address of current page pointer
uint8_t page_offset = 0;		// Offset from page base address

EEPROM_Status_e EEPROM_Increment_Page_Pointer(EEPROM_t *handler);
EEPROM_Status_e EEPROM_Clear_Page(EEPROM_t *handler, uint16_t page_base_addr);
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

#ifndef MEMORY_INITIALIZED
	uint8_t pointer_data[DATA_PAGES+1];
	pointer_data[0] = 0x0001;
	for(int i = 0; i<=DATA_PAGES; i++){
		pointer_data[i+1] = (DATA_BASE_ADDR + i*PAGE_SIZE)>>6;
	}
	if(EEPROM_Multi_Write(handler, pointer_data, POINTER_POINTER_ADDR, DATA_PAGES+1)!=EEPROM_OK){
		return EEPROM_ERR;
	}
#endif

	if(EEPROM_Random_Read(handler, &page_pointer_addr, POINTER_POINTER_ADDR)!=EEPROM_OK){
		return EEPROM_ERR;
	}
	EEPROM_Increment_Page_Pointer(handler);
	if(EEPROM_Random_Read(handler, &current_page_addr, page_pointer_addr)!=EEPROM_OK){
		return EEPROM_ERR;
	}
	ALIGN_ADDR(current_page_addr);

	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Increment_Page_Pointer(EEPROM_t *handler){
	page_pointer_addr++;
	if(!(page_pointer_addr % DATA_BASE_ADDR)){
		page_pointer_addr = POINTER_BASE_ADDR;
	}
	if(EEPROM_Write(handler, &page_pointer_addr, POINTER_POINTER_ADDR)!=EEPROM_OK){
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Log_Data(EEPROM_t *handler, uint8_t *data, uint8_t size){

	if(size > PAGE_SIZE){
		return EEPROM_ERR;
	}
	if(page_offset + size > PAGE_SIZE){
		uint16_t next_page_addr;
		uint16_t next_page_pointer_addr = page_pointer_addr + 1;
		if(!(next_page_pointer_addr) % DATA_BASE_ADDR){
			next_page_pointer_addr = POINTER_BASE_ADDR;
		}
		if(EEPROM_Random_Read(handler, &next_page_addr, next_page_pointer_addr)!=EEPROM_OK){
			return EEPROM_ERR;
		}
		ALIGN_ADDR(next_page_addr);
		if(EEPROM_Clear_Page(handler, next_page_addr)!=EEPROM_OK){
			return EEPROM_ERR;
		}
		uint8_t space_remaining = PAGE_SIZE-page_offset;
		if(EEPROM_Multi_Write(handler, data, current_page_addr|page_offset, space_remaining)!=EEPROM_OK){
			return EEPROM_ERR;
		}
		if(EEPROM_Multi_Write(handler, data + space_remaining, next_page_addr, size - space_remaining)!=EEPROM_OK){
			return EEPROM_ERR;
		}
		EEPROM_Increment_Page_Pointer(handler);
		page_offset = size - space_remaining;
	}
	else{
		if(page_offset == 0){
			if(EEPROM_Clear_Page(handler, current_page_addr)!=EEPROM_OK){
				return EEPROM_ERR;
			}
		}
		if(size > 1){
			if(EEPROM_Multi_Write(handler, data, current_page_addr|page_offset, size)!=EEPROM_OK){
				return EEPROM_ERR;
			}
		}
		else{
			if(EEPROM_Write(handler, data, current_page_addr|page_offset)!=EEPROM_OK){
				return EEPROM_ERR;
			}
		}
		page_offset += size;
		if(!(page_offset % PAGE_SIZE)){
			EEPROM_Increment_Page_Pointer(handler);
			if(EEPROM_Random_Read(handler, &current_page_addr, page_pointer_addr)!=EEPROM_OK){
				return EEPROM_ERR;
			}
			ALIGN_ADDR(current_page_addr);
			page_offset = 0;
		}
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Clear_Page(EEPROM_t *handler, uint16_t page_base_addr){
	uint8_t invalid_data[PAGE_SIZE];
	for(int i = 0; i < PAGE_SIZE - 1; i++){
		invalid_data[i] = 0xFF;
	}
	if(EEPROM_Multi_Write(handler, invalid_data, page_base_addr, PAGE_SIZE)!=EEPROM_OK){
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr){
	if(mem_addr > DATA_TOP_ADDR){
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
	if(mem_addr > DATA_TOP_ADDR){
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);		// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);	// Set low address byte
	for(int i = 0; i < size; i++){
		if(mem_addr % BUFFER_SIZE || i==0){
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
			buffer_index++;
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
	if(mem_addr > DATA_TOP_ADDR){
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
	if(mem_addr > DATA_TOP_ADDR || size > BUFFER_SIZE){
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
