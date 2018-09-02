#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"
#include "power.h"

static void _batt_window_load(Window *window);
static void _batt_window_unload(Window *window);
static void _draw_battery(Layer *layer, GContext *ctx);
static bool _visible = false;

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
    rcore_backlight_on(100, 3000);
}

static void _batt_window_unload(Window *window)
{
    notification_battery *nm = (notification_battery *)window->context;
    noty_free(window->context);
    _visible = false;
}

static void _draw_battery(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 20, DISPLAY_ROWS/2 - 32, 40, 64), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 10, DISPLAY_ROWS/2 - 40, 20, 8), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 30, 36, 60), 0, GCornerNone);
    
    switch (power_get_charge_mode())
    {
        case POWER_CHG_MODE_OFF: // not charging
//             notification_show_small_message("No Chg", frame);
            graphics_context_set_fill_color(ctx, GColorRed);
            graphics_context_set_stroke_color(ctx, GColorRed);
            graphics_context_set_stroke_width(ctx, 3);
            graphics_draw_circle(ctx, GPoint(DISPLAY_COLS/2, DISPLAY_ROWS/2), 14);
            graphics_draw_line(ctx, GPoint(DISPLAY_COLS/2 - 7, DISPLAY_ROWS/2 - 7),
                                    GPoint(DISPLAY_COLS/2 + 7, DISPLAY_ROWS/2 + 7));
            graphics_draw_line(ctx, GPoint(DISPLAY_COLS/2 - 7, DISPLAY_ROWS/2 + 7),
                                    GPoint(DISPLAY_COLS/2 + 7, DISPLAY_ROWS/2 - 7));
            break;
        case POWER_CHG_MODE_OVER_TEMP: // not charge over temp
//             notification_show_small_message("OT", frame);
            break;
        case POWER_CHG_MODE_PRE_CHG: // pre-charge
            graphics_context_set_fill_color(ctx, GColorRed);
            graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 10, 36, 10), 0, GCornerNone);
//             notification_show_small_message("p-chg", frame);
            break;
        case POWER_CHG_MODE_FAST_CHG_CC: // fast charge const cur
//             notification_show_small_message("f-c-cc", frame);
            graphics_context_set_fill_color(ctx, GColorGreen);
            graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 10, 36, 50), 0, GCornerNone);
            break;
        case POWER_CHG_MODE_FAST_CHG_CV: // fast charge const volt
            graphics_context_set_fill_color(ctx, GColorOrange);
            graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 15, 36, 30), 0, GCornerNone);
//             notification_show_small_message("f-c-cv", frame);
            break;
        case POWER_CHG_MODE_MAINT_CHG: // maint charge
            graphics_context_set_fill_color(ctx, GColorYellow);
            graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 24, 36, 54), 0, GCornerNone);
//             notification_show_small_message("maint", frame);
            break;
        case POWER_CHG_MODE_MAINT_CHG_DONE: // maint charge done
            graphics_context_set_fill_color(ctx, GColorGreen);
            graphics_fill_rect(ctx, GRect(DISPLAY_COLS/2 - 18, DISPLAY_ROWS/2 - 30, 36, 60), 0, GCornerNone);
//             notification_show_small_message("maint done", frame);
            break;
        case POWER_CHG_FAULT: // FAULT
//             notification_show_small_message("Charging", frame);
            break;
    }
}
