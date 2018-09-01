/* action_bar_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "node_list.h"
#include "status_bar_layer.h"
#include "appmanager.h"

static void _draw(Layer *layer, GContext *context);

static void _schedule_timer(StatusBarLayer* status_bar)
{
    TickType_t delay = pdMS_TO_TICKS(1000 * (60 - status_bar->last_time.tm_sec));
    status_bar->timer.when = xTaskGetTickCount() + delay;
    appmanager_timer_add(&status_bar->timer);
}

static void _timer_callback(CoreTimer* timer) {
    StatusBarLayer* status_bar = container_of(timer, StatusBarLayer, timer);

    memcpy(&status_bar->last_time, rebble_time_get_tm(), sizeof(struct tm));
    layer_mark_dirty(&status_bar->layer);

    _schedule_timer(status_bar);
}

void status_bar_layer_ctor(StatusBarLayer *status_bar)
{
    GRect frame = GRect(0, 0, DISPLAY_COLS, STATUS_BAR_LAYER_HEIGHT);
    layer_ctor(&status_bar->layer, frame);
    layer_set_update_proc(&status_bar->layer, _draw);
    layer_mark_dirty(&status_bar->layer);

    status_bar->background_color = GColorBlack;
    status_bar->foreground_color = GColorWhite;
    status_bar->separator_mode = StatusBarLayerSeparatorModeNone;
    status_bar->text = NULL;
    status_bar->timer.callback = _timer_callback;

    memcpy(&status_bar->last_time, rebble_time_get_tm(), sizeof(struct tm));
    _schedule_timer(status_bar);
}

void status_bar_layer_dtor(StatusBarLayer *sblayer)
{
    layer_dtor(&sblayer->layer);
}

StatusBarLayer *status_bar_layer_create(void)
{
    StatusBarLayer *status_bar = (StatusBarLayer*)app_calloc(1, sizeof(StatusBarLayer));    
    status_bar_layer_ctor(status_bar);
    
    return status_bar;
}

void status_bar_layer_destroy(StatusBarLayer *status_bar)
{
    status_bar_layer_dtor(status_bar);
    app_free(status_bar);
}

Layer *status_bar_layer_get_layer(StatusBarLayer *status_bar)
{
    return &status_bar->layer;
}

GColor status_bar_layer_get_background_color(StatusBarLayer *status_bar)
{
    return status_bar->background_color;
}

GColor status_bar_layer_get_foreground_color(StatusBarLayer *status_bar)
{
    return status_bar->foreground_color;
}

void status_bar_layer_set_colors(StatusBarLayer *status_bar, GColor background, GColor foreground)
{
    if (status_bar->background_color.argb != background.argb || status_bar->foreground_color.argb != foreground.argb) {
        status_bar->background_color = background;
        status_bar->foreground_color = foreground;
        
        layer_mark_dirty(&status_bar->layer);
    }
}

void status_bar_layer_set_separator_mode(StatusBarLayer *status_bar, StatusBarLayerSeparatorMode mode)
{
    if (status_bar->separator_mode != mode) {
        status_bar->separator_mode = mode;

        layer_mark_dirty(&status_bar->layer);
    }
}

void status_bar_layer_set_text(StatusBarLayer *status_bar, const char *status_text)
{
    status_bar->text = status_text;
    layer_mark_dirty(&status_bar->layer);
}

static void _draw(Layer *layer, GContext *context)
{
    StatusBarLayer *status_bar = (StatusBarLayer *) layer;
    GRect full_frame = layer_get_frame(layer);
    
    // Draw the background
    graphics_context_set_fill_color(context, status_bar->background_color);
    graphics_fill_rect(context, full_frame, 0, GCornerNone);

    if (status_bar->separator_mode == StatusBarLayerSeparatorModeDotted)
    {
        graphics_context_set_stroke_color(context, status_bar->foreground_color);
        for(int i = 0; i < full_frame.size.w; i += 2)
        {
            n_graphics_draw_pixel(context, n_GPoint(i, full_frame.size.h - 2));
        }
    }
    
    // Draw the text
    GRect text_frame = full_frame;
    text_frame.origin.y = full_frame.size.h - 18;
    graphics_context_set_text_color(context, status_bar->foreground_color);
    GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

    if (status_bar->text == NULL) {
        char time_string[8];
        strftime(time_string, 8, "%R", &status_bar->last_time);

        graphics_draw_text(context, time_string, text_font, text_frame,
                               GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
    }
    else {
        graphics_draw_text(context, status_bar->text, text_font, text_frame,
                               GTextOverflowModeTrailingEllipsis, n_GTextAlignmentCenter, 0);
    }
}
