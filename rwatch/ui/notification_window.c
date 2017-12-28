/* window.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include <stdbool.h>
#include "notification_window.h"
#include "librebble.h"
#include "ngfxwrap.h"

void notification_window_load(Window *window)
{
    Layer *layer = window_get_root_layer(window);
    
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    NotificationWindow *notification = (NotificationWindow *) window->user_data;
    
    Layer *main = layer_create(bounds);
    main->container = notification;
    layer_set_update_proc(main, notification_window_update_proc);
    layer_add_child(layer, main);
    
    layer_mark_dirty(layer);
    window_dirty(true);
}

void notification_window_update_proc(Layer *layer, GContext *ctx)
{
    NotificationWindow *notification = (NotificationWindow *) layer->container;
    int offset = notification->offset;
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    GBitmap *icon = notification->icon;
    char *app = notification->app_name;
    char *title = notification->title;
    char *body = notification->body;
    
    printf(app);
    
    // Draw the background:
    graphics_context_set_fill_color(ctx, GColorRed);
#ifdef PBL_RECT
    GRect rect_bounds = GRect(0, 0 - offset, bounds.size.w, 50);
    graphics_fill_rect_app(ctx, rect_bounds, 0, GCornerNone);
#else
    n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, -DISPLAY_COLS + 50), DISPLAY_COLS);
#endif
    
    // Draw the icon:
    if (icon != NULL)
    {
        GSize icon_size = icon->raw_bitmap_size;
        graphics_draw_bitmap_in_rect_app(ctx, icon, GRect(bounds.size.w / 2 - (icon_size.w / 2), 17 - offset - (icon_size.h / 2), icon_size.w, icon_size.h));
        gbitmap_destroy(icon);
    }
    
    GFont app_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    GFont body_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
    
#ifdef PBL_RECT
    GRect app_rect = GRect(10, 27 - offset, bounds.size.w - 20, 20);
    GRect title_rect = GRect(10, 50 - offset, bounds.size.w - 20, 30);
    GRect body_rect = GRect(10, 67 - offset, bounds.size.w - 20, 200);
#else
    GSize app_size = n_graphics_text_layout_get_content_size(app, app_font);
    GRect app_rect = GRect((bounds.size.w / 2) - (app_size.w / 2) - 10, 27 - offset, bounds.size.w - 20, 20);
    
    GSize title_size = n_graphics_text_layout_get_content_size(title, title_font);
    GRect title_rect = GRect((bounds.size.w / 2) - (title_size.w / 2) - 12, 50 - offset, bounds.size.w - 20, 20);
    
    //GSize body_size = n_graphics_text_layout_get_content_size(body, body_font);
    GRect body_rect = GRect(30, 67 - offset, bounds.size.w - 20, 200);
#endif
    
    // Draw the app:
    ctx->text_color = GColorWhite;
    graphics_draw_text_app(ctx, app, app_font, app_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the title:
    ctx->text_color = GColorBlack;
    graphics_draw_text_app(ctx, title, title_font, title_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the body:
    graphics_draw_text_app(ctx, body, body_font, body_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
}

void notification_window_unload(Window *window)
{
    
}

NotificationWindow* notification_window_create(const char *app_name, const char *title, const char *body, GBitmap *icon)
{
    NotificationWindow *notification = calloc(1, sizeof(NotificationWindow));
    
    if (notification == NULL)
    {
        SYS_LOG("notification_window", APP_LOG_LEVEL_ERROR, "No memory for NotificationWindow");
        return NULL;
    }
    
    notification->app_name = app_name;
    notification->title = title;
    notification->body = body;
    notification->icon = icon;
    notification->offset = 0;
    
    notification->window = window_create();
    notification->window->user_data = notification;
    
    window_set_window_handlers(notification->window, (WindowHandlers) {
        .load = notification_window_load,
        .unload = notification_window_unload,
    });
    
    return notification;
}

void window_stack_push_notification(NotificationWindow *notification)
{
    Window *window = notification_window_get_window(notification);
    window_stack_push(window, true);
}

Window* notification_window_get_window(NotificationWindow *notification_window)
{
    return notification_window->window;
}

