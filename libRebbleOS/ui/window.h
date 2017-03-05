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
#include <stdbool.h>
#include "point.h"
#include "rect.h"
#include "size.h"

struct Layer;

// typedef void (*WindowButtonEventHandler)(AppContextRef app_ctx, struct Window *window, PebbleButtonEvent *event);
// 
// typedef struct WindowInputHandlers
// {
//     struct 
//     {
//         WindowButtonEventHandler up;
//         WindowButtonEventHandler down;
//     } buttons;
// } WindowInputHandlers;

typedef void (*WindowHandler)(struct Window *window);

typedef struct WindowHandlers
{
    WindowHandler load;
    WindowHandler appear;
    WindowHandler disappear;
    WindowHandler unload;
} WindowHandlers;

typedef struct Window 
{
    Layer *root_layer;
    const GBitmap *status_bar_icon;
    //WindowInputHandlers input_handlers;
    WindowHandlers window_handlers;
    ClickConfigProvider click_config_provider; // callback to the provider function
    void *click_config_context;
    void *user_data;
    GColor background_color;
    bool is_render_scheduled;
    //bool on_screen : 1;
    bool is_loaded;
    //bool overrides_back_button : 1;
    //bool is_fullscreen : 1;
    //const char *debug_name;
} Window;


// Window management
Window *window_create();
void window_destroy(Window *window) ;
void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider);
void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context);
ClickConfigProvider window_get_click_config_provider(const Window *window);
void *window_get_click_config_context(Window *window);
void window_set_window_handlers(Window *window, WindowHandlers handlers);
Layer *window_get_root_layer(Window *window);
void window_set_background_color(Window *window, GColor background_color);
bool window_is_loaded(Window *window);
void window_set_user_data(Window *window, void *data);
void * window_get_user_data(const Window *window);
void window_single_click_subscribe(ButtonId button_id, ClickHandler handler);
void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);
void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);
void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);
void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context);
void window_set_click_context(ButtonId button_id, void *context);

void window_stack_push(Window *window, bool something);
void window_dirty(bool is_dirty);


void rbl_window_load_proc(void);
void rbl_window_load_click_config(void);
