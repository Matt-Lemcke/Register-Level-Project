/*
 * clocks.h
 *
 *  Created on: Sep 8, 2022
 *      Author: lemck
 */

#ifndef INC_CLOCKS_H_
#define INC_CLOCKS_H_

#include "stm32f429xx.h"

#define APB1_CLK_MHz        16
#define TIM7_PERIOD_MS		5000

typedef enum {
    SET,
    CLEAR
}Bit_State_e;

void SysClockConfig(void);
void Delay(uint32_t ms);
uint8_t Timer_Flag_Set(void);

#endif /* INC_CLOCKS_H_ */
