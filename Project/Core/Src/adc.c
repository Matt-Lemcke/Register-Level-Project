/*
 * adc.c
 *
 *  Created on: Sep. 26, 2022
 *      Author: lemck
 */


#include "adc.h"

#define ADC1_ENR	(1<<8)
#define ADC2_ENR	(1<<9)
#define ADC3_ENR	(1<<10)


ADC_Status_e ADC_Init(ADC_Handler_t *handler){

	// Configured for PC0, channel 10

	GPIO_Handle_t gpio;
	if(handler->adc == ADC1){
		RCC->APB2ENR |= ADC1_ENR;
	}
	else if(handler->adc == ADC2){
		RCC->APB2ENR |= ADC2_ENR;
	}
	else if(handler->adc == ADC3){
		RCC->APB2ENR |= ADC3_ENR;
	}
	else{
		return ADC_ERR;
	}

	ENABLE_GPIOC;
	gpio.port = GPIOC;
	gpio.mode = SET_MODE(0, MODE_ANALOG);
	gpio.out = SET_OUT(0, OUT_PUSH);
	gpio.alt = SET_AF(0, AF0);
	gpio.res = SET_RES(0, RES_NONE);
	gpio.speed = SET_SPEED(0, SPEED_LOW);
	if(GPIO_Init(&gpio) != GPIO_OK){
		return ADC_ERR;
	}
	handler->gpio = &gpio;

	ADC->CCR |= (1<<16);			// Prescalar = 4
	ADC->CCR |= (1<<23);			// Wake sensor
	handler->adc->CR1 |= (handler->resolution << 24);
	handler->adc->CR2 |= (1<<11);	// Left data allignment
	handler->adc->CR2 |= (1<<10);	// EOC enabled
	handler->adc->CR2 |= (1<<8);	// DMA enabled
	handler->adc->CR2 |= (1<<9);	// Continuous DMA
	handler->adc->CR2 |= (1<<1);	// Continuous conversion

	handler->adc->SMPR1 &= ~(7<<0);	// 3 cycle sampling

	handler->adc->SQR3 |= 10;		// Channel 10
	return ADC_OK;

}

ADC_Status_e ADC_Enable(ADC_Handler_t *handler){
	handler->adc->CR2 |= (1<<0);	// Turn on ADC
	Delay(1);
	return ADC_OK;
}

ADC_Status_e ADC_Configure_DMA(ADC_Handler_t *handler, uint16_t *buffer_location, uint16_t size){
	RCC->AHB1ENR |= (1<<22);	// Enable DMA2
	if(handler->adc == ADC1){
		// Channel 0, stream 0
		handler->dma = DMA2_Stream0;
		handler->dma->CR &= ~(7<<25);	// Set channel 0
	}
	else if(handler->adc == ADC2){
		// Channel 1, stream 2
		handler->dma = DMA2_Stream2;
		handler->dma->CR |= (1<<25);	// Set channel 1
	}
	else if(handler->adc == ADC3){
		// Channel 2, steam 0
		handler->dma = DMA2_Stream0;
		handler->dma->CR |= (2<<25);	// Set channel 2
	}
	else{
		return ADC_ERR;
	}

	handler->dma->CR |= (1<<9);		// Enable circular mode
	handler->dma->CR &= ~(3<<6);	// Peripheral -> memory transfer
	handler->dma->CR |= (1<<10);	// Enable memory increment
	handler->dma->CR |= (1<<11);	// 16 bit peripheral data
	handler->dma->CR |= (1<<13);	// 16 bit memory data

	handler->dma->NDTR = size;					// Size of data buffer
	handler->dma->PAR = &handler->adc->DR;		// Set peripheral address to read
	handler->dma->M0AR = buffer_location;		// Set buffer address in memory

	handler->dma->CR |= (1<<0);		// Enable DMA

	return ADC_OK;
}

ADC_Status_e ADC_Start(ADC_Handler_t *handler){
	handler->adc->SR = 0;			// Clear status flags
	handler->adc->CR2 |= (1<<30);	// Start conversions
	return ADC_OK;
}

uint16_t ADC_Read(ADC_Handler_t *handler){
	return (uint16_t)(handler->adc->DR & 0x0000FFFF);
}
