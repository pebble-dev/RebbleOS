#pragma once
/* notification_window.h
 *
 * Displays notifications sent to the watch from the phone.
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "status_bar_layer.h"
#include "property_animation.h"

typedef enum NotificationAction
{
    NotificationActionDismiss = 0,
    NotificationActionDismissAll = 1,
    NotificationActionReply = 2,
    NotificationActionCustom = 3
} NotificationAction;

typedef struct Notification
{
    GBitmap *icon;
    const char *app_name;
    const char *title;
    const char *body;
    char *custom_actions;
    GColor color;
    
    list_node node;
} Notification;

typedef struct NotificationLayer
{
    Layer layer;
    StatusBarLayer status_bar;
    
    uint16_t offset;
    uint16_t anim_offset_start;
    uint16_t anim_offset_goal;
    PropertyAnimation prop_anim;
    NotificationAction *actions;
    Notification *active;
    
    uint8_t notif_count;
    uint8_t selected_notif;
    list_head notif_list_head;
} NotificationLayer;

Notification* notification_create(const char *app_name, const char *title, const char *body, GBitmap *icon, GColor color);

static void notification_layer_ctor(NotificationLayer *notification_layer, GRect frame);
static void notification_layer_dtor(NotificationLayer *notification_layer);
NotificationLayer *notification_layer_create(GRect frame);
Layer* notification_layer_get_layer(NotificationLayer *notification_layer);
void notification_layer_destroy(NotificationLayer *notification_layer);
Notification* notification_create(const char *app_name, const char *title, const char *body, GBitmap *icon, GColor color);
void notification_layer_stack_push_notification(NotificationLayer *notification_layer, Notification *notification);
void notification_layer_configure_click_config(NotificationLayer *notification_layer, Window *window);
