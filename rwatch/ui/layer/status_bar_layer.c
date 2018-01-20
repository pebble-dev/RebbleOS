/* action_bar_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "utils.h"
#include "status_bar_layer.h"
#include "bitmap_layer.h"
#include "graphics.h"

typedef struct {
    int hours;
    int minutes;
} Time;

static Time s_last_time;

static void status_tick(struct tm *tick_time, TimeUnits tick_units)
{
    // Store time
    s_last_time.hours = tick_time->tm_hour % 12;
    s_last_time.minutes = tick_time->tm_min;
}

StatusBarLayer *status_bar_layer_create(void)
{
    StatusBarLayer *status_bar = (StatusBarLayer*)app_calloc(1, sizeof(StatusBarLayer));
    
    GRect frame = GRect(0, 0, 144, STATUS_BAR_LAYER_HEIGHT);
    
    Layer* layer = layer_create(frame);
    // give the layer a reference back to us
    layer->container = status_bar;
    status_bar->layer = layer;
    status_bar->background_color = GColorWhite;
    status_bar->text_color = GColorBlack;
    status_bar->separator_mode = StatusBarLayerSeparatorModeDotted;
    
    layer_set_update_proc(layer, draw);
    
    tick_timer_service_subscribe(MINUTE_UNIT, status_tick);

    layer_mark_dirty(layer);
    
    return status_bar;
}

void status_bar_layer_destroy(StatusBarLayer *status_bar)
{
    layer_destroy(status_bar);
    app_free(status_bar);
}

Layer *status_bar_layer_get_layer(StatusBarLayer *status_bar)
{
    return status_bar->layer;
}

GColor status_bar_layer_get_background_color(StatusBarLayer *status_bar)
{
    return status_bar->background_color;
}

GColor status_bar_layer_get_foreground_color(StatusBarLayer * status_bar)
{
    return status_bar->text_color;
}

void status_bar_layer_set_colors(StatusBarLayer * status_bar, GColor background, GColor foreground)
{
    status_bar->background_color = background;
    status_bar->text_color = foreground;
}

void status_bar_layer_set_separator_mode(StatusBarLayer * status_bar_layer, StatusBarLayerSeparatorMode mode)
{
    
}

static void draw(Layer *layer, GContext *context)
{
    StatusBarLayer *status_bar = (StatusBarLayer *) layer->container;
    GRect full_bounds = layer_get_bounds(layer);
    
    // Draw the background
    graphics_context_set_fill_color(context, status_bar->background_color);
    graphics_fill_rect(context, full_bounds, 0, GCornerNone);
    
    // Draw the time
    context->text_color = status_bar->text_color;
    GFont time_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    
    char time_string[8] = "";
    
    printf("%d:%02d", s_last_time.hours, s_last_time.minutes);
    snprintf(time_string, 8, "%d:%02d", s_last_time.hours, s_last_time.minutes);
    
    GRect text_bounds = GRect((full_bounds.size.w / 2) - 10, 0, 100, 10);
    graphics_draw_text_app(context, time_string, time_font, text_bounds, GTextOverflowModeTrailingEllipsis, n_GTextAlignmentLeft, 0);
    
    // TODO: Draw the separator
    if (status_bar->separator_mode == StatusBarLayerSeparatorModeDotted)
    {
        
    }
}
