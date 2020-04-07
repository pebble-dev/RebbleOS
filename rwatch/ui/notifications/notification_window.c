#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"

#define MODULE_NAME "notywin"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


static void _notif_man_window_load(Window *window);
static void _notif_man_window_unload(Window *window);
static void _nl_back_click_handler(ClickRecognizerRef _, void *context);
static bool _visible = false;
static NotificationLayer *_notif_layer;

void notification_message_display(OverlayWindow *overlay, Window *window)
{
    /* the overlay context has the message data 
     * lets take it with us */
    window->context = overlay->context;

    window_set_window_handlers(window, (WindowHandlers) {
        .load = _notif_man_window_load,
        .unload = _notif_man_window_unload,
    });
}


static void _notif_man_window_load(Window *window)
{
    window_set_background_color(window, GColorWhite);
    Layer *layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(layer);
    NotificationLayer *notif_layer = notification_layer_create(bounds);
    layer_add_child(layer, notification_layer_get_layer(notif_layer));

    notification_message *nm = (notification_message *)window->context;
    /* set the context to the layer now */
    nm->notification_layer = notif_layer;
    _notif_layer = notif_layer;

    notification_message *msg = (notification_message *)window->context;
    notification_layer_message_arrived(notif_layer, (Uuid *)msg->uuid);

    notification_layer_configure_click_config(notif_layer, window, _nl_back_click_handler);

    layer_mark_dirty(layer);
    window_dirty(true);
    _visible = true;
}

static void _notif_man_window_unload(Window *window)
{
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "O Unload %x", window);
    notification_message *nm = (notification_message *)window->context;
    notification_layer_destroy(nm->notification_layer);
    app_free(nm);
    _visible = false;
}

static void _nl_back_click_handler(ClickRecognizerRef _, void *context)
{
    assert(context);
    NotificationLayer *nl = (NotificationLayer *)context;
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "BC %x", nl);
    Window *w = layer_get_window(&nl->content_layer);
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "EBC %x", nl);
    notification_message *nm = (notification_message *)w->context;
    app_timer_reschedule(nm->data.timer, 10);
    SYS_LOG("NOTYM", APP_LOG_LEVEL_INFO, "EBC %x", nl);
//     app_timer_reschedule(nm->data.timer, 1);
//     overlay_window_destroy(nm->data.overlay_window);
//     app_free(context);
}

bool notification_window_overlay_visible(void)
{
    return _visible;
}

NotificationLayer *notification_window_get_layer(void)
{
    return _notif_layer;
}
