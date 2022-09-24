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
#define TIMEOUT_PERIOD_US   10


typedef enum {
    CLK_TIMEOUT,
    CLK_ERR,
    CLK_OK
}CLK_Status_e;

typedef enum {
    SET,
    CLEAR
}Bit_State_e;

void SysClockConfig(void);
void Delay(uint32_t ms);
void StartTimeout(uint32_t timeout_us);
uint8_t TimeoutFlag(void);
CLK_Status_e EndTimeout(void);

#endif /* INC_CLOCKS_H_ */
