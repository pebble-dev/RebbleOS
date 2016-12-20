#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"

typedef struct {
    uint16_t Pin;
    GPIO_TypeDef *Port;    
} button_t;

typedef struct {
    button_t Back;
    button_t Up;
    button_t Select;
    button_t Down;
} buttons_t;

void buttons_init(void);
uint8_t button_is_pressed(button_t *button);
void vButtonTask(void *pvParameters);
#endif
