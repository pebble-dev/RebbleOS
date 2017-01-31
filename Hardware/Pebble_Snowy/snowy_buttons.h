#ifndef __SNOWY_BUTTONS_H
#define __SNOWY_BUTTONS_H
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"

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

// API
uint8_t hw_button_pressed(button_t *button);
void hw_buttons_init(void);


void buttons_init_intn(uint32_t EXTIport, uint32_t EXTIline, uint32_t pinSource, uint32_t EXTI_irq);


#endif
