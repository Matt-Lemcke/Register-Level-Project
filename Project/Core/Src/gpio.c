/*
 * gpio.c
 *
 *  Created on: Sep. 15, 2022
 *      Author: lemck
 */
#include "gpio.h"

GPIO_Status_e GPIO_Init(GPIO_Handle_t *handle){

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

    handle->port->MODER = 0x55555555;
    handle->port->OTYPER = 0;
    handle->port->OSPEEDR = 0;
    handle->port->PUPDR = 0;
    handle->port->AFR[0] = 0;
    handle->port->AFR[1] = 0;
    GPIO_Clear(handle);

    return GPIO_OK;
}

void GPIO_Set(GPIO_Handle_t *handle, uint16_t pin_mask){
    handle->port->ODR |= pin_mask;
}

void GPIO_Reset(GPIO_Handle_t *handle, uint16_t pin_mask){
    handle->port->ODR &= ~pin_mask;
}

void GPIO_Clear(GPIO_Handle_t *handle){
    handle->port->ODR = 0;
}

// Example
//GPIO_Handle_t gpio_handler;
//ENABLE_GPIOB;
//gpio_handler.port = GPIOB;
//gpio_handler.mode = SET_MODE(8, MODE_AF) | SET_MODE(9, MODE_AF);
//gpio_handler.speed = SET_SPEED(8, SPEED_VHI) | SET_SPEED(9, SPEED_VHI);
//gpio_handler.out = SET_OUT(8, OUT_DRAIN) | SET_OUT(9, OUT_DRAIN);
//gpio_handler.res = SET_RES(8, RES_NONE) | SET_RES(9, RES_NONE);
//gpio_handler.alt = SET_AF(8, AF4) | SET_AF(9, AF4);
//GPIO_Init(&gpio_handler);
