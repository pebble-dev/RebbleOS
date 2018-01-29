/* notification.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "rebbleos.h"
#include "notification.h"
#include "notification_window.h"
#include "librebble.h"
#include "bitmap_layer.h"
#include "action_bar_layer.h"

const char *notif_name = "Notification";

static NotificationWindow *notif_window;

void notif_init(void)
{
    char *app = "RebbleOS";
    char *title = "Test Alert";
    char *body = "Testing a basic notification on RebbleOS. Create it using notification_window_create, with an app_name, title, body, and optional icon.";
    
    Notification *notification = notification_create(app, title, body, gbitmap_create_with_resource(77), GColorRed);
    
    window_stack_push_notification(notification);
    
    Notification *notification_two = notification_create("Discord", "Join Us", "Join us on the Pebble Discord in the #firmware channel", gbitmap_create_with_resource(20), GColorFromRGB(85, 0, 170));
    window_stack_push_notification(notification_two);
}

void notif_deinit(void)
{
    window_destroy(notif_window->window);
}

void notif_main(void)
{
    notif_init();
    app_event_loop();
    notif_deinit();
}
