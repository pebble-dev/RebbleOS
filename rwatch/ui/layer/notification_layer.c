/* notification_layer.c
 *
 * Displays notifications sent to the watch from the phone.
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include <stdbool.h>
#include "notification_layer.h"
#include "action_menu.h"
#include "status_bar_layer.h"
#include "property_animation.h"
#include "librebble.h"
#include "ngfxwrap.h"

static void _notification_layer_load(Window *window);
static void _notification_layer_unload(Window *window);
static void _notification_layer_update_proc(Layer *layer, GContext *ctx);
static void _click_config_provider(void *context);

static void notification_layer_ctor(NotificationLayer *notification_layer, GRect frame)
{
    layer_ctor(&notification_layer->layer, frame);
    list_init_head(&notification_layer->notif_list_head);
    layer_set_update_proc(&notification_layer->layer, _notification_layer_update_proc);
}

static void notification_layer_dtor(NotificationLayer *notification_layer)
{
    /* Free the chain */
    Notification *n;
    list_node *l = list_get_head(&notification_layer->notif_list_head);
    while(l)
    {
        n = list_elem(l, Notification, node);
        list_remove(&notification_layer->notif_list_head, &n->node);
        app_free(n);
        
        SYS_LOG("noty", APP_LOG_LEVEL_ERROR, "Deleted all Notifications");
        break;
    }
    layer_remove_from_parent(&notification_layer->status_bar.layer);
    status_bar_layer_destroy(&notification_layer->status_bar);
    layer_destroy(&notification_layer->layer);
    app_free(notification_layer);
    window_dirty(true);
}

static void _status_index_draw(Layer *layer, GContext *ctx)
{
    NotificationLayer *notification_layer = layer->callback_data;
    
    // Draw the status bar indicator
    char status[10];
    snprintf(status, 10, "%d/%d", notification_layer->selected_notif + 1, notification_layer->notif_count);
    ctx->text_color = status_bar_layer_get_foreground_color(&notification_layer->status_bar);
    GRect status_rect = GRect(0, -2, layer->bounds.size.w - 5, 14);
    graphics_draw_text(ctx, status, fonts_get_system_font(FONT_KEY_GOTHIC_14), status_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, 0);
}

NotificationLayer *notification_layer_create(GRect frame)
{
    SYS_LOG("noty", APP_LOG_LEVEL_INFO, "Created Notification Layer");

    /* Make the window */
    NotificationLayer *notification_layer = app_calloc(1, sizeof(NotificationLayer));
    notification_layer_ctor(notification_layer, frame);
    
    status_bar_layer_ctor(&notification_layer->status_bar);
    status_bar_layer_set_colors(&notification_layer->status_bar, notification_layer->active->color, GColorBlack);
    layer_add_child(&notification_layer->layer, status_bar_layer_get_layer(&notification_layer->status_bar));
    
#ifdef PBL_RECT
    Layer *status_index_layer = layer_create(status_bar_layer_get_layer(&notification_layer->status_bar)->frame);
    status_index_layer->callback_data = notification_layer;
    layer_set_update_proc(status_index_layer, _status_index_draw);
    layer_add_child(status_bar_layer_get_layer(&notification_layer->status_bar), status_index_layer);
#endif
    
    notification_layer->notif_count = 0;
    
    return notification_layer;
}

void notification_layer_destroy(NotificationLayer *notification_layer)
{
    notification_layer_dtor(notification_layer);    
}

Notification* notification_create(const char *app_name, const char *title, const char *body, GBitmap *icon, GColor color)
{
    Notification *notification = app_calloc(1, sizeof(Notification));
    
    if (notification == NULL)
    {
        SYS_LOG("notification_layer", APP_LOG_LEVEL_ERROR, "No memory for Notification");
        return NULL;
    }
    
    notification->app_name = app_name;
    notification->title = title;
    notification->body = body;
    notification->icon = icon;
    notification->color = color;
    
    return notification;
}

void notification_layer_stack_push_notification(NotificationLayer *notification_layer, Notification *notification)
{   
    list_init_node(&notification->node);
    list_insert_head(&notification_layer->notif_list_head, &notification->node);
    notification_layer->active = notification;
    
    notification_layer->notif_count += 1;
    
    notification_layer->offset = 0;
    status_bar_layer_set_colors(&notification_layer->status_bar, notification->color, GColorWhite);
    
    layer_mark_dirty(&notification_layer->layer);
}

Layer* notification_layer_get_layer(NotificationLayer *notification_layer)
{
    return &notification_layer->layer;
}

void notification_layer_configure_click_config(NotificationLayer *notification_layer, Window *window)
{
    window_set_click_config_provider_with_context(window, 
                                                  (ClickConfigProvider)_click_config_provider, 
                                                  notification_layer);
}

static void _set_scroll_offset(void *subject, int16_t value)
{
    NotificationLayer *notification_layer = container_of(subject, NotificationLayer, offset);
    notification_layer->offset = value;
    window_dirty(true);
}

static int16_t _get_scroll_offset(void *subject)
{
    NotificationLayer *notification_layer = container_of(subject, NotificationLayer, offset);
    return notification_layer->offset;
}

static const PropertyAnimationImplementation _scroll_impl = {
    .base = {
        .update = (AnimationUpdateImplementation) property_animation_update_int16,
    },
    .accessors = {
        .setter = { .int16 = _set_scroll_offset, },
        .getter = { .int16 = _get_scroll_offset, },
    },
};

static void _scroll_to(NotificationLayer *notification_layer, uint16_t start, uint16_t offset, bool animated)
{
    if (animated)
    {
        notification_layer->anim_offset_goal = offset;
        notification_layer->anim_offset_start = start;
        notification_layer->prop_anim = *property_animation_create(&_scroll_impl, &notification_layer->offset, &notification_layer->anim_offset_start, &notification_layer->anim_offset_goal);
        Animation *anim = property_animation_get_animation(&notification_layer->prop_anim);
        
#ifdef PBL_RECT
        animation_set_duration(anim, 200);
#else
        animation_set_duration(anim, 100);
#endif
        animation_schedule(anim);
    }
    else
    {
        notification_layer->offset = offset;
    }
}

static void _scroll_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;
    Notification *notification = notification_layer->active;
        
    if (notification->node.prev != &notification_layer->notif_list_head.node)
    {
        if (notification_layer->offset <= 1)
        {
            /* Show the previous notification on the stack */
            Notification *n = list_elem(notification->node.prev, Notification, node);
            notification_layer->active = n;
            
            // Scroll to the bottom
            _scroll_to(notification_layer, 0, DISPLAY_ROWS, true);
#ifdef PBL_RECT
            status_bar_layer_set_colors(&notification_layer->status_bar, n->color, GColorWhite);
#else
            status_bar_layer_set_colors(&notification_layer->status_bar, GColorWhite, GColorBlack);
#endif
            notification_layer->selected_notif -= 1;
        }
        else
        {
            _scroll_to(notification_layer, DISPLAY_ROWS, 0, true);
#ifndef PBL_RECT
                status_bar_layer_set_colors(&notification_layer->status_bar, notification->color, GColorWhite);
#endif
        }
    }
    else
    {
        _scroll_to(notification_layer, 20, 0, true);
#ifndef PBL_RECT
        status_bar_layer_set_colors(&notification_layer->status_bar, notification->color, GColorWhite);
#endif
    }
    
    window_dirty(true);
}

static void _scroll_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;
    Notification *notification = notification_layer->active;
    
    /* we reached the bottom */
    Notification *n = list_elem(list_get_tail(&notification_layer->notif_list_head), Notification, node);
    if (notification != n)
    {
        if (notification_layer->offset > 0)
        {
            /* Show the next notification on the stack */
            Notification *n = list_elem(notification->node.next, Notification, node);
            notification_layer->active = n;

            SYS_LOG("notification_layer", APP_LOG_LEVEL_DEBUG, notification_layer->active->app_name);
            
            // Scroll to top
            _scroll_to(notification_layer, DISPLAY_ROWS, 0, true);
            
            status_bar_layer_set_colors(&notification_layer->status_bar, n->color, GColorWhite);
            
            notification_layer->selected_notif += 1;
        } else {
            _scroll_to(notification_layer, DISPLAY_ROWS, DISPLAY_ROWS - 20, true);
        #ifndef PBL_RECT
            status_bar_layer_set_colors(&notification_layer->status_bar, GColorWhite, GColorBlack);
        #endif
        }
    }
    else
    {
        _scroll_to(notification_layer, DISPLAY_ROWS - 20, DISPLAY_ROWS, true);
#ifndef PBL_RECT
        status_bar_layer_set_colors(&notification_layer->status_bar, GColorWhite, GColorBlack);
#endif
    }
    window_dirty(true);
}

static void _action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
    // ACTION!
}

static void _show_options_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;
    Notification *notification = notification_layer->active;
    
    // Make + show the ActionMenu
    ActionMenuLevel *root_level = action_menu_level_create(3);
    ActionMenuLevel *reply_level = action_menu_level_create(3);
    
    action_menu_level_add_child(root_level, reply_level, "Reply");
    action_menu_level_add_action(root_level, "Dismiss", _action_performed_callback, NULL);
    action_menu_level_add_action(root_level, "Dismiss All", _action_performed_callback, NULL);
    
    action_menu_level_add_action(reply_level, "Dictate", _action_performed_callback, NULL);
    action_menu_level_add_action(reply_level, "Canned Response", _action_performed_callback, NULL);
    action_menu_level_add_action(reply_level, "Emoji", _action_performed_callback, NULL);
    
    ActionMenuConfig config = (ActionMenuConfig)
    {
        .root_level = root_level,
        .colors = {
            .background = notification_layer->active->color,
            .foreground = GColorWhite,
        },
        .align = ActionMenuAlignCenter
    };
    
    action_menu_open(&config);
}

static void _pop_notification_click_handler(ClickRecognizerRef recognizer, void *context)
{
  /* XXX pop the window, call the unload handler on the notification_layer */
}

static void _click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) _scroll_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) _scroll_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) _show_options_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) _pop_notification_click_handler);
    
    window_set_click_context(BUTTON_ID_DOWN, context);
    window_set_click_context(BUTTON_ID_UP, context);
    window_set_click_context(BUTTON_ID_SELECT, context);
}

static void _notification_layer_update_proc(Layer *layer, GContext *ctx)
{
    NotificationLayer *notification_layer = container_of(layer, NotificationLayer, layer);
    assert(notification_layer);
    Notification *notification = notification_layer->active;
    if (!notification)
        return;
    int offset = -STATUS_BAR_LAYER_HEIGHT + notification_layer->offset; // offset for the status_bar
    GRect bounds = layer_get_unobstructed_bounds(layer);
    
    GBitmap *icon = notification->icon;
    char *app = notification->app_name;
    char *title = notification->title;
    char *body = notification->body;
    
    // Draw the background:
    graphics_context_set_fill_color(ctx, notification->color);
#ifdef PBL_RECT
    GRect rect_bounds = GRect(0, 0 - offset, bounds.size.w, 35);
    graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);
#else
    n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, (-DISPLAY_COLS + 35) - offset), DISPLAY_COLS);
#endif
    
    // Draw the icon:
    if (icon != NULL)
    {
        GSize icon_size = icon->raw_bitmap_size;
#ifdef PBL_RECT
        graphics_draw_bitmap_in_rect(ctx, icon, GRect(5, 17 - offset - (icon_size.h / 2), icon_size.w, icon_size.h));
#else
        graphics_draw_bitmap_in_rect(ctx, icon, GRect(bounds.size.w / 2 - (icon_size.w / 2), 17 - offset - (icon_size.h / 2), icon_size.w, icon_size.h));
#endif
    }
    
#ifdef PBL_RECT
    GRect app_rect = GRect(35, -offset, bounds.size.w - 35, 20);
    GRect title_rect = GRect(10, 35 - offset, bounds.size.w - 20, 30);
    GRect body_rect = GRect(10, 50 - offset, bounds.size.w - 20, (DISPLAY_ROWS * 2) - 67);
    
    GTextAlignment alignment = GTextAlignmentLeft;
#else
    GRect app_rect = GRect(0, 30 - offset, bounds.size.w, 20);
    
    GRect title_rect = GRect(0, 52 - offset, bounds.size.w, 20);
    
    //GSize body_size = n_graphics_text_layout_get_content_size(body, body_font);
    GRect body_rect = GRect(0, 67 - offset, bounds.size.w, (DISPLAY_ROWS * 2) - 50);
    
    GTextAlignment alignment = GTextAlignmentCenter;
#endif
    
    // Draw the app:
#ifdef PBL_RECT
    ctx->text_color = GColorWhite;
#else
    ctx->text_color = notification->color;
#endif
    graphics_draw_text(ctx, app, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), app_rect, GTextOverflowModeTrailingEllipsis, alignment, 0);
    
    // Draw the title:
    ctx->text_color = GColorBlack;
    graphics_draw_text(ctx, title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), title_rect, GTextOverflowModeTrailingEllipsis, alignment, 0);
    
    // Draw the body:
    graphics_draw_text(ctx, body, fonts_get_system_font(FONT_KEY_GOTHIC_24), body_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    
    // Draw the indicator:
    graphics_context_set_fill_color(ctx, GColorBlack);
    n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS + 2, DISPLAY_ROWS / 2), 10);
    
    // And the other one:
#ifndef PBL_RECT
    if (notification->node.next != NULL && notification_layer->offset > 0)
    {
        Notification *n = list_elem(notification->node.next, Notification, node);
        graphics_context_set_fill_color(ctx, n->color);
        n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, DISPLAY_ROWS - 2), 10);
    }
#endif

}
