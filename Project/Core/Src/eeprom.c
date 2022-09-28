/*
 * eeprom.c
 *
 *  Created on: Sep. 20, 2022
 *      Author: lemck
 */


#include "eeprom.h"

// Comment out to reinitialize memory and page pointers
#define MEMORY_INITIALIZED

// EEPROM memory pages
#define PAGE_SIZE	64
#define DATA_PAGES	126

// Memory addresses
#define POINTER_POINTER_ADDR	0x0000
#define POINTER_BASE_ADDR	0x0001
#define DATA_BASE_ADDR		0x0080
#define DATA_TOP_ADDR		0x1FFF

// Align address read from EEPROM page pointer
#define ALIGN_ADDR(addr)		(addr=(addr & 0x00FF)<<6)

uint16_t current_page_addr;		// Current page address
uint16_t page_pointer_addr;		// Address of current page pointer
uint8_t page_offset = 0;		// Offset from page base address

// Private function definition
EEPROM_Status_e EEPROM_Increment_Page_Pointer(EEPROM_t *handler);
EEPROM_Status_e EEPROM_Clear_Page(EEPROM_t *handler, uint16_t page_base_addr);
EEPROM_Status_e EEPROM_Random_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr);
EEPROM_Status_e EEPROM_Current_Read(EEPROM_t *handler, uint8_t *data);
EEPROM_Status_e EEPROM_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr);
EEPROM_Status_e EEPROM_Multi_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size);
EEPROM_Status_e EEPROM_Sequential_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size);

/*   ----------------------
*	Public Functions
*/   ----------------------

EEPROM_Status_e EEPROM_Init(EEPROM_t *handler){

	if(I2C_Test_Device(handler->i2c, handler->address)!=I2C_OK){					// Test device connection
		return EEPROM_NO_DEVICE;
	}

#ifndef MEMORY_INITIALIZED
	uint8_t pointer_data[DATA_PAGES+1];
	pointer_data[0] = 0x0001;									// Reset address of current page pointer
	for(int i = 0; i<=DATA_PAGES; i++){
		pointer_data[i+1] = (DATA_BASE_ADDR + i*PAGE_SIZE)>>6;					// Initialize addresses in page pointers
	}
	if(EEPROM_Multi_Write(handler, pointer_data, POINTER_POINTER_ADDR, DATA_PAGES+1)!=EEPROM_OK){	// Write page pointers to EEPROM pointer section
		return EEPROM_ERR;
	}
#endif

	if(EEPROM_Random_Read(handler, &page_pointer_addr, POINTER_POINTER_ADDR)!=EEPROM_OK){		// Read last used page pointer
		return EEPROM_ERR;
	}
	EEPROM_Increment_Page_Pointer(handler);								// Move to next page pointer
	if(EEPROM_Random_Read(handler, &current_page_addr, page_pointer_addr)!=EEPROM_OK){		// Read address of the current page
		return EEPROM_ERR;
	}
	ALIGN_ADDR(current_page_addr);
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Log_Data(EEPROM_t *handler, uint8_t *data, uint8_t size){

	// Check that data does not exceed page length
	if(size > PAGE_SIZE){
		return EEPROM_ERR;
	}
	
	// If data write will cross page boundaries
	if(page_offset + size > PAGE_SIZE){
		uint16_t next_page_addr;
		uint16_t next_page_pointer_addr = page_pointer_addr + 1;
		if(!(next_page_pointer_addr) % DATA_BASE_ADDR){
			next_page_pointer_addr = POINTER_BASE_ADDR;
		}
		if(EEPROM_Random_Read(handler, &next_page_addr, next_page_pointer_addr)!=EEPROM_OK){				// Retrieve address of next page to write to
			return EEPROM_ERR;
		}
		ALIGN_ADDR(next_page_addr);
		if(EEPROM_Clear_Page(handler, next_page_addr)!=EEPROM_OK){							// Clear the next page
			return EEPROM_ERR;
		}
		uint8_t space_remaining = PAGE_SIZE-page_offset;
		if(EEPROM_Multi_Write(handler, data, current_page_addr|page_offset, space_remaining)!=EEPROM_OK){		// Write data into remaining space of current page
			return EEPROM_ERR;
		}
		if(EEPROM_Multi_Write(handler, data + space_remaining, next_page_addr, size - space_remaining)!=EEPROM_OK){	// Write data into the next page
			return EEPROM_ERR;
		}
		EEPROM_Increment_Page_Pointer(handler);										// Move to next page
		page_offset = size - space_remaining;										// Update current offset for new page
	}
	
	// If page write will not cross boundaries
	else{
		if(page_offset == 0){
			if(EEPROM_Clear_Page(handler, current_page_addr)!=EEPROM_OK){						// Clear page if starting on new page
				return EEPROM_ERR;
			}
		}
		if(size > 1){
			if(EEPROM_Multi_Write(handler, data, current_page_addr|page_offset, size)!=EEPROM_OK){			// Write data to current page
				return EEPROM_ERR;
			}
		}
		else{
			if(EEPROM_Write(handler, data, current_page_addr|page_offset)!=EEPROM_OK){
				return EEPROM_ERR;
			}
		}
		page_offset += size;												// Update offset from page base address
		if(!(page_offset % PAGE_SIZE)){
			EEPROM_Increment_Page_Pointer(handler);									// Move to next page if reached end of current page
			if(EEPROM_Random_Read(handler, &current_page_addr, page_pointer_addr)!=EEPROM_OK){
				return EEPROM_ERR;
			}
			ALIGN_ADDR(current_page_addr);
			page_offset = 0;
		}
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Sequential_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size){
	
	if(mem_addr > DATA_TOP_ADDR || size > BUFFER_SIZE){			// Check for valid input params
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);			// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);			// Set low address byte
	if(I2C_Write(handler->i2c, handler->address, 2, NO_STOP)!=I2C_OK){	// Set address in device
		return EEPROM_ERR;
	}
	if(I2C_Read(handler->i2c, handler->address, size)!=I2C_OK){		// Read contents into buffer
		return EEPROM_ERR;
	}
	while(size>0){
		data[size-1] = handler->i2c->buffer[size-1];			// Move read data from buffer
		size--;
	}
	return EEPROM_OK;
}

/*   ----------------------
*	Private Functions
*/   ----------------------
	
EEPROM_Status_e EEPROM_Clear_Page(EEPROM_t *handler, uint16_t page_base_addr){
	uint8_t invalid_data[PAGE_SIZE];
	for(int i = 0; i < PAGE_SIZE - 1; i++){							// Set all data to invalid entry (0xFF)
		invalid_data[i] = 0xFF;
	}
	if(EEPROM_Multi_Write(handler, invalid_data, page_base_addr, PAGE_SIZE)!=EEPROM_OK){	// Write invalid data to entire page
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr){
	
	if(mem_addr > DATA_TOP_ADDR){						// Check that address is valid
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);			// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);			// Set low address byte
	handler->i2c->buffer[2] = *data;					// Set data byte
	if(I2C_Write(handler->i2c, handler->address, 3, STOP)!=I2C_OK){		// Write to device
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Multi_Write(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr, uint8_t size){
	uint8_t buffer_index = 0;
	if(mem_addr > DATA_TOP_ADDR){						// Check that address is valid
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);			// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);			// Set low address byte
	for(int i = 0; i < size; i++){
		if(mem_addr % BUFFER_SIZE || i==0){
			handler->i2c->buffer[buffer_index+2] = data[i];		// Add data to buffer
			mem_addr++;
			buffer_index++;
		}
		else{
			if(I2C_Write(handler->i2c, handler->address, buffer_index+2, STOP)!=I2C_OK){	// Write to device when data fills buffer
				return EEPROM_ERR;
			}
			buffer_index = 0;
			handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);				// Set high address byte
			handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);				// Set low address byte
			handler->i2c->buffer[buffer_index+2] = data[i];					// Continue adding remaining data to buffer
			mem_addr++;
			buffer_index++;
		}
	}
	if(I2C_Write(handler->i2c, handler->address, buffer_index+2, STOP)!=I2C_OK){	// Write to device
		return EEPROM_ERR;
	}
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Current_Read(EEPROM_t *handler, uint8_t *data){
	if(I2C_Read(handler->i2c, handler->address, 1)!=I2C_OK){	// Read from current set address in device
		return EEPROM_ERR;
	}
	*data = handler->i2c->buffer[0];				// Read from buffer
	return EEPROM_OK;
}

EEPROM_Status_e EEPROM_Random_Read(EEPROM_t *handler, uint8_t *data, uint16_t mem_addr){
	if(mem_addr > DATA_TOP_ADDR){						// Check that address is valid
		return EEPROM_ERR;
	}
	handler->i2c->buffer[0] = (uint8_t)(mem_addr >> 4);			// Set high address byte
	handler->i2c->buffer[1] = (uint8_t)(mem_addr & 0x00FF);			// Set low address byte
	if(I2C_Write(handler->i2c, handler->address, 2, NO_STOP)!=I2C_OK){	// Set address in device
		return EEPROM_ERR;
	}
	if(I2C_Read(handler->i2c, handler->address, 1)!=I2C_OK){		// Read byte from set address into buffer
		return EEPROM_ERR;
	}
	*data = handler->i2c->buffer[0];					// Read from buffer
	return EEPROM_OK;
}


