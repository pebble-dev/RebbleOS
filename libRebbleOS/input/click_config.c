/* click_config.c
 * routines for [...]
 * RebbleOS core
 *
 * Author: Barry Carter <barry.carter@gmail.com>
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
