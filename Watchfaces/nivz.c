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
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "math.h"


// const char *app_name = "Simple";

static void nivz_update_proc(Layer *layer, GContext *ctx);
void nivz_main(void);
void nivz_init(void);
void nivz_deinit(void);
void nivz_tick(struct tm *tick_time, TimeUnits tick_units);

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_text_layer;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

static void nivz_window_load(Window *window)
{
    printf("WF load\n");
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, nivz_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    s_text_layer = text_layer_create(bounds);
    // TODO until we compile those fonts in, this wil crash
    // as we have no font set
    //layer_add_child(window_layer, s_text_layer);
    //text_layer_set_text(s_text_layer, "Hello\n");

    tick_timer_service_subscribe(SECOND_UNIT, nivz_tick);
    layer_mark_dirty(s_canvas_layer);
}


static void nivz_window_unload(Window *window)
{
    layer_destroy(s_canvas_layer);
}

// tick
void nivz_tick(struct tm *tick_time, TimeUnits tick_units)
{   
    printf("appmain\n");
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.minutes = tick_time->tm_min;

    
    // Redraw
    if (s_canvas_layer)
    {
        layer_mark_dirty(s_canvas_layer);
    }
}

void nivz_init(void)
{
    printf("init\n");
    s_main_window = window_create();
    
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = nivz_window_load,
        .unload = nivz_window_unload,
    });
    
    window_stack_push(s_main_window, true);
}

void nivz_deinit(void)
{
    window_destroy(s_main_window);
}

void nivz_main(void)
{
    nivz_init();
    app_event_loop();
    nivz_deinit();
}


const uint8_t digits[10][16] = {
    {1, 1, 1, 
     1, 0, 1,
     1, 0, 1,
     1, 0, 1,
     1, 1, 1},
    {0, 0, 1, 
     0, 0, 1,
     0, 0, 1,
     0, 0, 1,
     0, 0, 1},
    {1, 1, 1, 
     0, 0, 1,
     1, 1, 1,
     1, 0, 0,
     1, 1, 1},
    {1, 1, 1, 
     0, 0, 1,
     1, 1, 1,
     0, 0, 1,
     1, 1, 1},
    {1, 0, 1, 
     1, 0, 1,
     1, 1, 1,
     0, 0, 1,
     0, 0, 1},
    {1, 1, 1, 
     1, 0, 0,
     1, 1, 1,
     0, 0, 1,
     1, 1, 1},
    {1, 1, 1, 
     1, 0, 0,
     1, 1, 1,
     1, 0, 1,
     1, 1, 1},
    {1, 1, 1, 
     0, 0, 1,
     0, 0, 1,
     0, 0, 1,
     0, 0, 1},
    {1, 1, 1, 
     1, 0, 1,
     1, 1, 1,
     1, 0, 1,
     1, 1, 1},
    {1, 1, 1, 
     1, 0, 1,
     1, 1, 1,
     0, 0, 1,
     1, 1, 1}
    };


// Function to draw a digit in the graphics context at a given co-ordinate
void draw_digit(GContext *nGContext, uint8_t d, uint8_t x, uint8_t y)
{
    uint8_t s_color_channels[3];
    graphics_context_set_fill_color(nGContext, GColorWhite);

    // loop through the 5 rows
    for (uint8_t ii=0; ii<5; ii++)
    {
        // loop through the 3 columns
        for (uint8_t jj=0; jj<3; jj++)
        {
            // use the 1 or 0 to decide if we need to draw a square or not
            if(digits[d][(ii*3)+jj] == 1)
            {
                
                for (int i = 0; i < 3; i++)
                {
                    s_color_channels[i] = rand() % 256;
                }
                graphics_context_set_fill_color(nGContext, GColorFromRGB(s_color_channels[0], s_color_channels[1], s_color_channels[2]));
                GRect rect = GRect((jj*15)+x,(ii*15)+y, 13, 14);
                graphics_fill_rect(nGContext, rect, 0, GCornerNone);
            }
        }
    }
}


static void nivz_update_proc(Layer *layer, GContext *nGContext)
{
  // Get the time
  uint8_t hour = s_last_time.hours;
  uint8_t minute = s_last_time.minutes;
  
  GRect full_bounds = layer_get_bounds(layer);
  // Clear the screen
  graphics_context_set_fill_color(nGContext, GColorBlack);
  graphics_fill_rect(nGContext, full_bounds, 0, GCornerNone);
  
  // Draw the Hours 
  draw_digit(nGContext, (int)(hour/10), (full_bounds.size.w/2) - 52, (full_bounds.size.h/2) - 82);
  draw_digit(nGContext, (int)(hour%10), (full_bounds.size.w/2) + 8, (full_bounds.size.h/2) - 82);
  
  //Draw the Minutes
  draw_digit(nGContext, (int)(minute/10), (full_bounds.size.w/2) - 52, (full_bounds.size.h/2) + 8);
  draw_digit(nGContext, (int)(minute%10), (full_bounds.size.w/2) + 8, (full_bounds.size.h/2) + 8);
  
}
