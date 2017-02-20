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

Window *top_window;

// window stuff
Window *window_create()
{
    Window *window = calloc(1, sizeof(Window));
    if (window == NULL)
    {
        printf("No memory for Window\n");
        return NULL;
    }
    // and it's root layer
    GRect bounds = GRect(0, 0, 144, 168);
    
    window->root_layer = layer_create(bounds);
        
    return window;
}

void window_set_window_handlers(Window *window, WindowHandlers handlers)
{
    if (window == NULL)
        return;
    
    window->window_handlers = handlers;
}

void window_stack_push(Window *window, bool something)
{
    top_window = window;
}

void window_destroy(Window *window)
{
    // free all of the layers
    layer_destroy(window->root_layer);
    // and now the window
    free(window);
}

Layer *window_get_root_layer(Window *window)
{
    if (window == NULL)
        return NULL;
    
    return window->root_layer;
}

void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider)
{
    window->click_config_provider = click_config_provider;
}

void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context)
{
    window->click_config_provider = click_config_provider;
    window->click_config_context = context;
}

ClickConfigProvider window_get_click_config_provider(const Window *window)
{
    return window->click_config_provider;
}

void *window_get_click_config_context(Window *window)
{
    return window->click_config_context;
}

void window_set_background_color(Window *window, GColor background_color)
{
    window->background_color = background_color;
}

bool window_is_loaded(Window *window)
{
    return window->is_loaded;
}

void window_set_user_data(Window *window, void *data)
{
    window->user_data = data;
}

void * window_get_user_data(const Window *window)
{
    return window->user_data;
}

void window_single_click_subscribe(ButtonId button_id, ClickHandler handler)
{
}

void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler)
{
}

void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler)
{
}

void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler)
{
}

void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context)
{
}

void window_set_click_context(ButtonId button_id, void *context)
{
}


void window_dirty(bool is_dirty)
{
    top_window->is_render_scheduled = is_dirty;
    walk_layers(top_window->root_layer);
    rbl_draw();
    top_window->is_render_scheduled = false;
}




// call the load callback
void rbl_window_load_proc(void)
{
    // TODO
    // we are not tracking app root windows yet, just share out the top_window for now
    if (top_window->window_handlers.load)
        top_window->window_handlers.load(top_window);
}
