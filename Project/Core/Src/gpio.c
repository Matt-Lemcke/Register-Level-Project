/*
 * gpio.c
 *
 *  Created on: Sep. 15, 2022
 *      Author: lemck
 */
#include "gpio.h"

GPIO_Status_e GPIO_Init(GPIO_Handle_t *handle){

    // Initialize gpio port settings
    handle->port->MODER = handle->mode;
    handle->port->OTYPER = handle->out;
    handle->port->OSPEEDR = handle->speed;
    handle->port->PUPDR = handle->res;
    handle->port->AFR[0] = handle->alt & 0x00000000FFFFFFFF;
    handle->port->AFR[1] = handle->alt >> 32;
    GPIO_Clear(handle);

    return GPIO_OK;
}

GPIO_Status_e GPIO_Deinit(GPIO_Handle_t *handle){

    // Reset gpio port settings
    handle->port->MODER = 0x55555555;   // Mode reset value
    handle->port->OTYPER = 0;
    handle->port->OSPEEDR = 0;
    handle->port->PUPDR = 0;
    handle->port->AFR[0] = 0;
    handle->port->AFR[1] = 0;
    GPIO_Clear(handle);

    return GPIO_OK;
}

void GPIO_Set(GPIO_Handle_t *handle, uint16_t pin_mask){
    // Turn on gpio pins from pin mask
    handle->port->ODR |= pin_mask;
}

void GPIO_Reset(GPIO_Handle_t *handle, uint16_t pin_mask){
    // Turn off gpio pins from pin mask
    handle->port->ODR &= ~pin_mask;
}

void GPIO_Clear(GPIO_Handle_t *handle){
    // Reset all gpio pin outputs
    handle->port->ODR = 0;
}

