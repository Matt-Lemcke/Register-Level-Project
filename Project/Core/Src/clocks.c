/*
 * clocks.c
 *
 *  Created on: Sep 8, 2022
 *      Author: lemck
 */

#include "clocks.h"

volatile uint32_t timeout_count;
volatile uint8_t timeout_flag = 0;

void SysTimeoutConfig(uint16_t tick_us);

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
    SysTimeoutConfig(TIMEOUT_PERIOD_US);
}

// tick_us is the time interval in microseconds when the timer triggers an update interrupt
void SysTimeoutConfig(uint16_t tick_us){
    RCC->APB1ENR |= (1<<5);     // Enable Timer 7 clock

    TIM7->DIER |= (1<<0);       // Enable update interrupt

    // CLK Speed: 16Mhz -> PSC -> 1MHz
    TIM7->PSC = APB1_CLK_MHz;   // PSC = 16M/1M = 16

    // Tick Speed: tick_us/1Mhz
    TIM7->ARR = tick_us;

    // Configure interrupt
    NVIC_SetPriority(TIM7_IRQn, 0);
    NVIC_EnableIRQ(TIM7_IRQn);
}


void StartTimeout(uint32_t timeout_us){
	TIM7->CR1 |= (1<<0);	// Enable timer
	timeout_count = timeout_us/TIMEOUT_PERIOD_US;
	timeout_flag = 0;
}

CLK_Status_e EndTimeout(void){
	TIM7->CR1 &= ~(1<<0);	//Stop timer
	if(timeout_flag){
		timeout_flag = 0;
		return CLK_TIMEOUT;
	}
	else{
		return CLK_OK;
	}

}

uint8_t TimeoutFlag(void){
	return timeout_flag;
}

void Delay(uint32_t ms){
	uint32_t time = 1000*APB1_CLK_MHz*ms;
	while(time--);
}

void TIM7_IRQHandler(){
	if(timeout_count){
		timeout_count--;
	}
	else{
		timeout_flag = 1;
	}
    TIM7->SR &= ~(1<<0);	// Clear interrupt flag
}
