/* progress_overlay.c
 * Draws a progress bar
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "platform_res.h"
#include "notification_manager.h"

/* Configure Logging */
#define MODULE_NAME "progwnd"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE


static void _progress_window_load(Window *window);
static void _progress_window_unload(Window *window);
static void _progress_window_draw(Layer *layer, GContext *ctx);
static bool _visible = false;
static uint32_t _total_bytes;
static uint32_t _progress_bytes;

void progress_window_update_arrived(notification_progress *progress)
{
    _progress_bytes = progress->progress_bytes;
    _total_bytes = progress->total_bytes;
}

void progress_window_overlay_display(OverlayWindow *overlay, Window *window)
{       
    notification_progress *prog = (notification_progress *)window->context;

    if (_visible)
    {
        /* if we are already visible, just request a redraw */
        window_dirty(true);
        return;
    }
    _visible = true;
   
    window_set_window_handlers(window, (WindowHandlers) {
        .load = _progress_window_load,
        .unload = _progress_window_unload,
    });
    
    progress_window_update_arrived(prog);
}

void progress_window_overlay_destroy(OverlayWindow *overlay, Window *window)
{
    notification_progress *prog = (notification_progress *)window->context;
}

bool progress_window_visible(void)
{
    return _visible;
}

static void _progress_window_load(Window *window)
{
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    layer_set_update_proc(layer, _progress_window_draw);
    notification_load_click_config(window);
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _progress_window_unload(Window *window)
{
    _visible = false;
}

static void _progress_window_draw(Layer *layer, GContext *ctx)
{
    GRect wrect = layer->window->frame;
    notification_progress *prog = (notification_progress *)layer->window->context;
    
    if (!_visible)
        return;
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, wrect, 0, GCornerNone);
    ctx->text_color = GColorWhite;
    GRect r = GRect(wrect.origin.x, wrect.origin.y + 20, wrect.size.w, wrect.size.h);
    char buf[100];
    
    if (_progress_bytes < _total_bytes)
    {
        snprintf(buf, 100, "Downloaded: %d/%d", _progress_bytes, _total_bytes);
        graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
    }
    else
    {
        snprintf(buf, 100, "Done", _progress_bytes, _total_bytes);
        graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
        notification_reschedule_timer(layer->window, 500);
        _visible = false;
    }
}

