#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"
#include "notification_window.h"

#define MODULE_NAME "notywin"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


static void _notif_man_window_load(Window *window);
static void _notif_man_window_unload(Window *window);
static void _nl_back_click_handler(ClickRecognizerRef _, void *context);
static bool _visible = false;

static NotificationWindow _notif_window;

void notification_message_display(OverlayWindow *overlay, Window *window) {
    /* the overlay context has the message data 
     * lets take it with us */
    notification_window_ctor(&_notif_window, window);
    
    notification_message *msg = overlay->context;
    notification_window_set_notifications(&_notif_window, (Uuid *)msg->uuid, 1, 0);
    
    _visible = true;
    
    /* back click handler needs to dtor notification_window_ctor and clear _visible */
}

void notification_message_destroy(OverlayWindow *overlay, Window *window) {
    notification_window_dtor(&_notif_window);
    free(overlay->context);
    _visible = false;
}


bool notification_window_overlay_visible(void) {
    return _visible;
}

NotificationWindow *notification_window_get(void) {
    return &_notif_window;
}
