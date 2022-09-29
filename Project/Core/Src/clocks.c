/*
 * clocks.c
 *
 *  Created on: Sep 8, 2022
 *      Author: lemck
 */

#include "clocks.h"

volatile uint8_t timer_flag_set = 0;

void SysTimerConfig(uint16_t tick_ms);

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
    SysTimerConfig(TIM7_PERIOD_MS);
}

// tick_us is the time interval in microseconds when the timer triggers an update interrupt
void SysTimerConfig(uint16_t tick_ms){
    RCC->APB1ENR |= (1<<5);     // Enable Timer 7 clock

    TIM7->DIER |= (1<<0);       // Enable update interrupt

    // CLK Speed: 16Mhz -> PSC -> 1kHz
    TIM7->PSC = 1000*APB1_CLK_MHz;   // PSC = 16M/1k = 16*1000

    // Tick Speed: tick_us/1Mhz
    TIM7->ARR = tick_ms;

    // Configure interrupt
    NVIC_SetPriority(TIM7_IRQn, 0);
    NVIC_EnableIRQ(TIM7_IRQn);

    TIM7->CR1 |= (1<<0);	// Enable timer
}

uint8_t Timer_Flag_Set(void){
	if(timer_flag_set){
		timer_flag_set = 0;
		return 1;
	}
	else{
		return 0;
	}
}

void Delay(uint32_t ms){
	uint32_t time = 1000*APB1_CLK_MHz*ms;
	while(time--);
}

void TIM7_IRQHandler(){
	timer_flag_set = 1;
    TIM7->SR &= ~(1<<0);	// Clear interrupt flag
}
