/*
 * adc.h
 *
 *  Created on: Sep. 26, 2022
 *      Author: lemck
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdlib.h>
#include "stm32f429xx.h"
#include "gpio.h"
#include "clocks.h"

#define RES_12_BIT	0
#define RES_10_BIT	1
#define RES_8_BIT	2
#define RES_6_BIT	3

typedef enum {
	ADC_OK,
	ADC_ERR
}ADC_Status_e;

typedef struct {
	ADC_TypeDef	*adc;
	uint8_t resolution;

	GPIO_Handle_t *gpio;
	DMA_Stream_TypeDef *dma;
}ADC_Handler_t;

ADC_Status_e ADC_Init(ADC_Handler_t *handler);
ADC_Status_e ADC_Enable(ADC_Handler_t *handler);
ADC_Status_e ADC_Start(ADC_Handler_t *handler);
uint16_t ADC_Read(ADC_Handler_t *handler);
ADC_Status_e ADC_Configure_DMA(ADC_Handler_t *handler, uint16_t *buffer_location, uint16_t size);

#endif /* INC_ADC_H_ */
