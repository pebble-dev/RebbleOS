/* notification.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "rebbleos.h"
#include "notification.h"
#include "notification_layer.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "action_bar_layer.h"
#include "platform_res.h"

static NotificationLayer* _notif_layer;
static Window* _notif_window;

static void _notif_window_load(Window *window);
static void _notif_window_unload(Window *window);

void notif_init(void)
{
    _notif_window = window_create();
    
    window_set_window_handlers(_notif_window, (WindowHandlers) {
        .load = _notif_window_load,
        .unload = _notif_window_unload,
    });
    
    window_stack_push(_notif_window, true);
}

static void _notif_window_load(Window *window)
{
    char *app = "RebbleOS";
    char *title = "Test Alert";
    char *body = "Testing a basic notification on RebbleOS. Create it using notification_window_create, with an app_name, title, body, and optional icon.";
    
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    _notif_layer = notification_layer_create(bounds);
    Notification *notification = notification_create(app, title, body, gbitmap_create_with_resource(RESOURCE_ID_ROUNDRECT_THING), GColorRed);

    notification_layer_stack_push_notification(_notif_layer, notification);
    
    Notification *notification_two = notification_create("Discord", "Join Us", "Join us on the Pebble Discord in the #firmware channel", gbitmap_create_with_resource(RESOURCE_ID_ALARM_BELL_RINGING), GColorFromRGB(85, 0, 170));
    notification_layer_stack_push_notification(_notif_layer, notification_two);

    layer_add_child(layer, notification_layer_get_layer(_notif_layer));
        
    notification_layer_configure_click_config(_notif_layer, window);
    
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _notif_window_unload(Window *window)
{
    notification_layer_destroy(_notif_layer);
}

void notif_deinit(void)
{
//     notification_window_destroy(notif_window);
    window_destroy(_notif_window);
}

void notif_main(void)
{
    notif_init();
    app_event_loop();
    notif_deinit();
}
