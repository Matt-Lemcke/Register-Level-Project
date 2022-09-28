/*
 * tmp_sensor.c
 *
 *  Created on: Sep. 27, 2022
 *      Author: lemck
 */

#include "tmp_sensor.h"
#include "adc.h"

uint16_t adc_buffer[BUFFER_SIZE];

ADC_Handler_t adc_handle;

float TMP_Conversion(float volts);

TMP_Status_e TMP_Init(void){

	adc_handle.adc = ADC1;
	adc_handle.resolution = RES_8_BIT;
	if(ADC_Init(&adc_handle) != ADC_OK){
		return TMP_ERR;;
	}
	if(ADC_Enable(&adc_handle) != ADC_OK){
		return TMP_ERR;
	}
	if(ADC_Configure_DMA(&adc_handle, adc_buffer, BUFFER_SIZE) != ADC_OK){
		return TMP_ERR;
	}
	if(ADC_Start(&adc_handle) != ADC_OK){
		return TMP_ERR;
	}
	return TMP_OK;
}

uint8_t TMP_Read_Volts(void){
	uint16_t data;
	for(int i = 0; i<BUFFER_SIZE; i++){
		data += (adc_buffer[i]>>8);	// Realign data bits
	}
	data = data/BUFFER_SIZE;
	return (uint8_t)data;
}

float TMP_Conversion(float volts){
	float celcius = (3*volts/255 - 0.75)*100 + 25;	// From datasheet
	return celcius;
}
