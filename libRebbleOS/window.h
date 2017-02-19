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

typedef void (*WindowHandlerLoadUnloadProc)(struct Window *window);
typedef struct WindowHandlers
{
    WindowHandlerLoadUnloadProc load;
    WindowHandlerLoadUnloadProc unload;
} WindowHandlers;

typedef struct Window 
{
    Layer *root_layer;
    //const GBitmap *status_bar_icon;
    //WindowInputHandlers input_handlers;
    WindowHandlers window_handlers;
    //ClickConfigProvider click_config_provider;
    //void *click_config_context;
    //void *user_data;
    //GColor background_color : 2;
    //bool is_render_scheduled : 1;
    //bool on_screen : 1;
    //bool is_loaded : 1;
    //bool overrides_back_button : 1;
    //bool is_fullscreen : 1;
    //const char *debug_name;

    bool dirty;
} Window;


// Window management
Window *window_create();
void window_set_window_handlers(Window *window, WindowHandlers handlers);
void window_stack_push(Window *window, bool something);
void window_destroy(Window *window);
Layer *window_get_root_layer(Window *window);
void window_dirty(bool is_dirty);
