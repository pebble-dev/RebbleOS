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

#include "librebble.h"
#include "click_config.h"

typedef struct ButtonHolder {
    ButtonId button_id;
    ClickConfig click_config;
} ButtonHolder;

// we only have 4 buttons, so we can get away with a quick array lookup
static ButtonHolder click_config[NUM_BUTTONS];

void window_single_click_subscribe(ButtonId button_id, ClickHandler handler)
{
    click_config[button_id].click.handler = handler;
    click_config[button_id].click.repeat_interval_ms = 0;
}

void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);
void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);
void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);
void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context);

void window_invoke_single_click_cb(Window *window)
{
    if (window == null || window->click_config_provider == null)
        return;
    
    window->click_config_provider(window);
}/*

// when a button is pressed, it proxies to here
void click_config_trigger(ButtonId button_id, bool up)
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        if (click_config[i].click.handler && up == false)
            click_config[i].click.handler(context);
        
        if (click_config[i].multi_click.handler)
    }
}


void click_config_add(ButtonId button_id, ClickConfig click_config)
{
    
}*/
