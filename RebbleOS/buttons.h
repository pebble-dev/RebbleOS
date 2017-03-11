#pragma once
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
#include "FreeRTOS.h"
// not ideal. TODO reorg
#include "rebbleos.h"
#include "librebble.h"

#define butDEBOUNCE_DELAY       ( 1 / portTICK_RATE_MS )

#define BUTTON_STATE_PRESSED    0
#define BUTTON_STATE_RELEASED   1
#define BUTTON_STATE_LONG       2
#define BUTTON_STATE_REPEATING  3
#define BUTTON_STATE_MULTI      4
#define BUTTON_STATE_MULTI_DONE 5

typedef struct ButtonHolder {
    uint8_t button_id;
    ClickConfig click_config;
    TickType_t repeat_time;
    TickType_t press_time;
    uint8_t state;
} ButtonHolder;

void buttons_init(void);
uint8_t button_pressed(ButtonId button_id);
void button_isr(ButtonId button_id);

void button_single_click_subscribe(ButtonId button_id, ClickHandler handler);
void button_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);
void button_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);
void button_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);
void button_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context);
void button_unsubscribe_all(void);

ButtonHolder *button_add_click_config(ButtonId button_id, ClickConfig click_config);
