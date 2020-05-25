/* single_notification_layer.h
 * Renders a single notification.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */
 
#pragma once
#include "layer.h"
#include "gbitmap.h"
 
 typedef struct SingleNotificationLayer {
    Layer layer;
    char *title, *subtitle, *body;
    const char *source;
    char *timestamp;
    GBitmap *icon;
} SingleNotificationLayer;

extern void single_notification_layer_ctor(SingleNotificationLayer *l, GRect frame);
extern void single_notification_layer_dtor(SingleNotificationLayer *l);
extern void single_notification_layer_set_notification(SingleNotificationLayer *l, rebble_notification *notif);
extern Layer *single_notification_layer_get_layer(SingleNotificationLayer *l);
extern uint16_t single_notification_layer_height(SingleNotificationLayer *l);
