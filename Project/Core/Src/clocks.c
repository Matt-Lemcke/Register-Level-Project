/*
 * clocks.c
 *
 *  Created on: Sep 8, 2022
 *      Author: lemck
 */

#include "clocks.h"


/*
 * System clock: 8 MHz
 * FLCK cortex clock: 16 MHz
 * APB1 Peripheral clock: 16 MHz
 * APB2 Peripheral clock: 16 MHz
 */

void SysClockConfig(void){

	// Configure prescalars
	RCC->CFGR &= ~ (1<<7);	// AHB /1
	RCC->CFGR &= ~ (1<<12);	// APB1 /1
	RCC->CFGR &= ~ (1<<15);	// APB2 /1

	// CMSIS system configuration for STM32F429
	SystemInit();
	SystemCoreClockUpdate();
}
