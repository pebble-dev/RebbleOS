/* notification_window.c
 *
 * Displays notifications sent to the watch from the phone.
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include <stdbool.h>
#include "notification_window.h"
#include "action_menu.h"
#include "status_bar_layer.h"
#include "librebble.h"
#include "ngfxwrap.h"

// XXX TODO nofifications don't free memory
static NotificationWindow *notification_window;
static StatusBarLayer *status_bar;

static void scroll_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    Notification *notification = notification_window->active;
    
    if (notification_window->offset == 0 && notification->next != NULL)
    {
        // Show the next notification on the stack
        notification_window->active = notification->next;
        
        // Scroll to bottom
        notification_window->offset = DISPLAY_ROWS;
        
#ifdef PBL_RECT
        status_bar_layer_set_colors(status_bar, notification_window->active->color, GColorWhite);
#else
        status_bar_layer_set_colors(status_bar, GColorWhite, GColorBlack);
#endif
    }
    else
    {
        notification_window->offset = 0;
        status_bar_layer_set_colors(status_bar, notification_window->active->color, GColorWhite);
    }
    
    window_dirty(true);
}

static void scroll_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    Notification *notification = notification_window->active;
    
    if (notification_window->offset == DISPLAY_ROWS && notification->previous != NULL)
    {
        // Show the previous notification on the stack
        notification_window->active = notification->previous;
        SYS_LOG("notification_window", APP_LOG_LEVEL_DEBUG, notification->previous->app_name);
        
        // Scroll to top
        notification_window->offset = 0;
        status_bar_layer_set_colors(status_bar, notification_window->active->color, GColorWhite);
    }
    else
    {
        notification_window->offset = DISPLAY_ROWS;
        
#ifndef PBL_RECT
        status_bar_layer_set_colors(status_bar, GColorWhite, GColorBlack);
#endif
    }
    
    window_dirty(true);
}

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
    // ACTION!
}

static void show_options_click_handler(ClickRecognizerRef recognizer, void *context)
{
    Notification *notification = (Notification *) context;
    
    // Make + show the ActionMenu
    ActionMenuLevel *root_level = action_menu_level_create(3);
    ActionMenuLevel *reply_level = action_menu_level_create(3);
    
    action_menu_level_add_child(root_level, reply_level, "Reply");
    action_menu_level_add_action(root_level, "Dismiss", action_performed_callback, NULL);
    action_menu_level_add_action(root_level, "Dismiss All", action_performed_callback, NULL);
    
    action_menu_level_add_action(reply_level, "Dictate", action_performed_callback, NULL);
    action_menu_level_add_action(reply_level, "Canned Response", action_performed_callback, NULL);
    action_menu_level_add_action(reply_level, "Emoji", action_performed_callback, NULL);
    
    ActionMenuConfig config = (ActionMenuConfig)
    {
        .root_level = root_level,
        .colors = {
            .background = notification_window->active->color,
            .foreground = GColorWhite,
        },
        .align = ActionMenuAlignCenter
    };
    
    action_menu_open(&config);
}

static void pop_notification_click_handler(ClickRecognizerRef recognizer, void *context)
{
    /* Close the NotificationWindows (ALL OF THEM)
    if (notification_window->active == notification)
    {
        appmanager_app_start("System");
    }
    else
    {
        // Pop windows off the stack
        window_stack_pop(true);
        window_dirty(true);
    } */
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) scroll_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) scroll_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) show_options_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) pop_notification_click_handler);
    
    window_set_click_context(BUTTON_ID_DOWN, context);
    window_set_click_context(BUTTON_ID_UP, context);
    window_set_click_context(BUTTON_ID_SELECT, context);
}

void notification_window_load(Window *window)
{
    Layer *layer = window_get_root_layer(window);
    
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    Layer *main = layer_create(bounds);
    layer_set_update_proc(main, notification_window_update_proc);
    layer_add_child(layer, main);
    
    // Set the click config provider
    window_set_click_config_provider_with_context(window, (ClickConfigProvider) click_config_provider, notification_window);
    
    // Status Bar
    status_bar = status_bar_layer_create();
    status_bar_layer_set_colors(status_bar, notification_window->active->color, GColorWhite);
    layer_add_child(main, status_bar_layer_get_layer(status_bar));
    
    layer_mark_dirty(layer);
    window_dirty(true);
}

void notification_window_update_proc(Layer *layer, GContext *ctx)
{
    Notification *notification = notification_window->active;
    int offset = -4 + notification_window->offset; // -4 because of the status_bar
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    GBitmap *icon = notification->icon;
    char *app = notification->app_name;
    char *title = notification->title;
    char *body = notification->body;
    
    SYS_LOG("notification_window", APP_LOG_LEVEL_DEBUG, app);
    
    // Draw the background:
    graphics_context_set_fill_color(ctx, notification->color);
#ifdef PBL_RECT
    GRect rect_bounds = GRect(0, 0 - offset, bounds.size.w, 35);
    graphics_fill_rect_app(ctx, rect_bounds, 0, GCornerNone);
#else
    n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, (-DISPLAY_COLS + 35) - offset), DISPLAY_COLS);
#endif
    
    // Draw the icon:
    if (icon != NULL)
    {
        GSize icon_size = icon->raw_bitmap_size;
        graphics_draw_bitmap_in_rect_app(ctx, icon, GRect(bounds.size.w / 2 - (icon_size.w / 2), 17 - offset - (icon_size.h / 2), icon_size.w, icon_size.h));
    }
    
#ifdef PBL_RECT
    GRect app_rect = GRect(10, 35 - offset, bounds.size.w - 20, 20);
    GRect title_rect = GRect(10, 52 - offset, bounds.size.w - 20, 30);
    GRect body_rect = GRect(10, 67 - offset, bounds.size.w - 20, (DISPLAY_ROWS * 2) - 67);
    
    GTextAlignment alignment = GTextAlignmentLeft;
#else
    GRect app_rect = GRect(0, 35 - offset, bounds.size.w, 20);
    
    GRect title_rect = GRect(0, 52 - offset, bounds.size.w, 20);
    
    //GSize body_size = n_graphics_text_layout_get_content_size(body, body_font);
    GRect body_rect = GRect(0, 67 - offset, bounds.size.w, (DISPLAY_ROWS * 2) - 50);
    
    GTextAlignment alignment = GTextAlignmentCenter;
#endif
    
    // Draw the app:
    ctx->text_color = notification->color;
    graphics_draw_text_app(ctx, app, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), app_rect, GTextOverflowModeTrailingEllipsis, alignment, 0);
    
    // Draw the title:
    ctx->text_color = GColorBlack;
    graphics_draw_text_app(ctx, title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), title_rect, GTextOverflowModeTrailingEllipsis, alignment, 0);
    
    // Draw the body:
    graphics_draw_text_app(ctx, body, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), body_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the indicator:
    graphics_context_set_fill_color(ctx, GColorBlack);
    n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS + 2, DISPLAY_ROWS / 2), 10);
    
    // And the other one:
#ifndef PBL_RECT
    if (notification->previous != NULL && notification_window->offset > 0)
    {
        graphics_context_set_fill_color(ctx, notification->previous->color);
        n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, DISPLAY_ROWS - 2), 10);
    }
#endif
}

void notification_window_unload(Window *window)
{
    
}

Notification* notification_create(const char *app_name, const char *title, const char *body, GBitmap *icon, GColor color)
{
    Notification *notification = app_calloc(1, sizeof(Notification));
    
    if (notification == NULL)
    {
        SYS_LOG("notification_window", APP_LOG_LEVEL_ERROR, "No memory for Notification");
        return NULL;
    }
    
    notification->app_name = app_name;
    notification->title = title;
    notification->body = body;
    notification->icon = icon;
    notification->color = color;
    
    return notification;
}

void window_stack_push_notification(Notification *notification)
{
    if (notification_window == NULL || notification_window->window == NULL)
    {
        // Make the window
        notification_window = app_calloc(1, sizeof(NotificationWindow));
        notification_window->window = window_create();
        
        window_set_window_handlers(notification_window->window, (WindowHandlers) {
            .load = notification_window_load,
            .unload = notification_window_unload,
        });
    }
    
    notification->previous = notification_window->active;
    notification_window->active->next = notification;
    notification_window->active = notification;
    
    SYS_LOG("notification_window", APP_LOG_LEVEL_DEBUG, "PUSHING NOTIF %s ON TOP OF %s", notification->app_name, notification->previous->app_name);
    
    // Show the notification_window if not already pushed
    if (!window_stack_contains_window(notification_window->window))
    {
        window_stack_push(notification_window->window, true);
    }
    
    window_dirty(true);
}

Window* notification_window_get_window(NotificationWindow *notification_window)
{
    return notification_window->window;
}

