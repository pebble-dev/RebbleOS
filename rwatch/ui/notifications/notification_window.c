#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"

static void _notif_man_window_load(Window *window);
static void _notif_man_window_unload(Window *window);
static void _nl_back_click_handler(ClickRecognizerRef _, void *context);

void notification_message_display(OverlayWindow *overlay, Window *window)
{    
    if (message_count() == 0)
    {
        SYS_LOG("NOTY", APP_LOG_LEVEL_ERROR, "No Messages?");
        return;
    }
    
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
    notification_message *message = (notification_message *)window->context;
    
    char *app = "RebbleOS";
    char *title = "Test Alert";
    window->background_color = GColorWhite;
    
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    NotificationLayer *notif_layer = notification_layer_create(bounds);
    
    cmd_phone_attribute_t *a;
    list_foreach(a, &message->message->attributes_list_head, cmd_phone_attribute_t, node)
    {
        Notification *notification = notification_create(app, title, (char*)a->data, gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE), GColorRed);        
        notification_layer_stack_push_notification(notif_layer, notification);
    }
        
    layer_add_child(layer, notification_layer_get_layer(notif_layer));

    notification_layer_configure_click_config(notif_layer, window, _nl_back_click_handler);

    /* set the context to the layer now */

    message->notification_layer = notif_layer;
    
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _notif_man_window_unload(Window *window)
{
    notification_message *nm = (notification_message *)window->context;
    notification_layer_destroy(nm->notification_layer);
    
    
}

static void _nl_back_click_handler(ClickRecognizerRef _, void *context)
{
    NotificationLayer *nl = (NotificationLayer *)context;
    notification_message *nm = (notification_message *)nl->layer.window->context;
    overlay_window_destroy(nm->data.overlay_window);
    noty_free(context);
}
