
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
#include "stdio.h"
#include "string.h"
#include "math.h"


// const char *app_name = "Simple";

static void simple_update_proc(Layer *layer, GContext *nGContext);
void simple_main(void);
void simple_init(void);
void simple_deinit(void);

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_text_layer;

uint8_t s_color_channels[3];

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

static void simple_window_load(Window *window)
{
    printf("WF load\n");
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, simple_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    s_text_layer = text_layer_create(bounds);
    // TODO until we compile those fonts in, this wil crash
    // as we have no font set
    //layer_add_child(window_layer, s_text_layer);
    //text_layer_set_text(s_text_layer, "Hello\n");

    //tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}


static void simple_window_unload(Window *window)
{
    layer_destroy(s_canvas_layer);
}

void simple_init(void)
{
    printf("init\n");
    s_main_window = window_create();
    
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = simple_window_load,
        .unload = simple_window_unload,
    });
    
    window_stack_push(s_main_window, true);
}

void simple_main(void)
{
    simple_init();
    app_event_loop();
    simple_deinit();
}


void simple_deinit(void)
{
    window_destroy(s_main_window);
}


void simple_tick(void)
{
    struct tm *tick_time = rbl_get_tm();
    
    printf("appmain\n");
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
    s_last_time.minutes = tick_time->tm_min;

    for (int i = 0; i < 3; i++)
    {
        s_color_channels[i] = rand() % 256;
    }

    // Redraw
    if (s_canvas_layer)
    {
        layer_mark_dirty(s_canvas_layer);
    }
}

#define HAND_MARGIN 10

static void simple_update_proc(Layer *layer, GContext *nGContext)
{   
    GRect full_bounds = layer_get_bounds(layer);
    //graphics_context_set_fill_color(nGContext, GColorWhite);
    graphics_context_set_fill_color(nGContext, GColorFromRGB(s_color_channels[0], s_color_channels[1], s_color_channels[2]));
     
    graphics_fill_rect(nGContext, full_bounds, 0, GCornerNone);
    
    graphics_context_set_stroke_color(nGContext, GColorBlack);
    graphics_context_set_stroke_width(nGContext, 4);
    //graphics_context_set_antialiased(nGContext, ANTIALIASING);
 
    uint8_t s_radius = 60;
    
    graphics_context_set_fill_color(nGContext, GColorWhite);
    //GPoint s_center = { .x = 70, .y = 90 };
    GPoint s_center = grect_center_point(&full_bounds);
    graphics_fill_circle(nGContext, s_center, s_radius);
 
    graphics_draw_circle(nGContext, s_center, s_radius);
     
    // Plot hands
    GPoint minute_hand = (GPoint) {
        .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * s_last_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.x,
        .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * s_last_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.y,
    };
    
    GPoint hour_hand = (GPoint) {
        .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * s_last_time.hours / 12) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
        .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * s_last_time.hours / 12) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
    };

    // Draw hands with positive length only
    if (s_radius > 2 * HAND_MARGIN) {
        graphics_context_set_stroke_color(nGContext, GColorRed);
        graphics_draw_line(nGContext, s_center, hour_hand);
    }
    if (s_radius > HAND_MARGIN) {
        graphics_context_set_stroke_color(nGContext, GColorBlack);
        graphics_draw_line(nGContext, s_center, minute_hand);
    }    
}

