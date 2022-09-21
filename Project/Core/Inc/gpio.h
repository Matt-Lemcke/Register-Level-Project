/*
 * gpio.h
 *
 *  Created on: Sep. 15, 2022
 *      Author: lemck
 */

#ifndef INC_GPIO_H_
#define INC_GPIO_H_


#include "stm32f429xx.h"
#include <stdlib.h>

#define ENABLE_GPIOA RCC->AHB1ENR |= (1<<0)
#define ENABLE_GPIOB RCC->AHB1ENR |= (1<<1)
#define ENABLE_GPIOC RCC->AHB1ENR |= (1<<2)
#define ENABLE_GPIOD RCC->AHB1ENR |= (1<<3)
#define ENABLE_GPIOE RCC->AHB1ENR |= (1<<4)
#define ENABLE_GPIOF RCC->AHB1ENR |= (1<<5)
#define ENABLE_GPIOG RCC->AHB1ENR |= (1<<6)
#define ENABLE_GPIOH RCC->AHB1ENR |= (1<<7)

#define PIN(pin)                (1<<pin)
#define SET_MODE(pin, val)      (val<<(pin<<1))
#define SET_SPEED(pin, val)     (val<<(pin<<1))
#define SET_OUT(pin, val)       (val<<pin)
#define SET_RES(pin, val)       (val<<(pin<<1))
#define SET_AF(pin, val)        ((uint64_t)val<<((uint64_t)pin<<2))


#define MODE_INPUT  0x0
#define MODE_OUTPUT 0x1
#define MODE_AF     0x2
#define MODE_ANALOG 0x3

#define SPEED_LOW   0x0
#define SPEED_MED   0x1
#define SPEED_HI    0x2
#define SPEED_VHI   0x3

#define RES_NONE    0x0
#define RES_PUP  	0x1
#define RES_PDOWN	0x2

#define OUT_PUSH    0x0
#define OUT_DRAIN   0x1

#define AF0         0x0
#define AF1         0x1
#define AF2         0x2
#define AF3         0x3
#define AF4         0x4
#define AF5         0x5
#define AF6         0x6
#define AF7         0x7
#define AF8         0x8
#define AF9         0x9
#define AF10        0xA
#define AF11        0xB
#define AF12        0xC
#define AF13        0xD
#define AF14        0xE
#define AF15        0xF

typedef enum {
    GPIO_OK,
    GPIO_ERR
}GPIO_Status_e;

typedef struct {
    uint32_t mode;
    uint32_t speed;

    uint16_t out;
    uint32_t res;
    
    uint64_t alt;
    
    GPIO_TypeDef *port;
}GPIO_Handle_t;

GPIO_Status_e GPIO_Init(GPIO_Handle_t *handle);
GPIO_Status_e GPIO_Deinit(GPIO_Handle_t *handle);
void GPIO_Set(GPIO_Handle_t *handle, uint16_t pin_mask);
void GPIO_Reset(GPIO_Handle_t *handle, uint16_t pin_mask);
void GPIO_Clear(GPIO_Handle_t *handle);

#endif /* INC_GPIO_H_ */
