#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"

static void _batt_window_load(Window *window);
static void _batt_window_unload(Window *window);
static void _draw_battery(Layer *layer, GContext *ctx);

void battery_overlay_display(OverlayWindow *overlay, Window *window)
{
    if (_visible)
    {
        /* if we are already visible, just request a redraw */
        window_dirty(true);
        return;
    }
    _visible = true;

    window_set_window_handlers(window, (WindowHandlers) {
        .load = _batt_window_load,
        .unload = _batt_window_unload,
    });
}

bool battery_overlay_visible(void)
{
    return _visible;
}

void battery_overlay_destroy(OverlayWindow *overlay, Window *window)
{      

}

static void _batt_window_load(Window *window)
{
    notification_battery *message = (notification_battery*)window->context;
        
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    layer_set_update_proc(layer, _draw_battery);
   
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _batt_window_unload(Window *window)
{
    notification_battery *nm = (notification_battery *)window->context;
    noty_free(window->context);
}

static void _draw_battery(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 20, DISPLAY_ROWS/2 - 32, 40, 64), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 10, DISPLAY_ROWS/2 - 40, 20, 8), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 30, 36, 60), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorGreen);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 10, 36, 40), 0, GCornerNone);
}
