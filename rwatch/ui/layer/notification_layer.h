#pragma once
/* notification_window.h
 *
 * Displays notifications sent to the watch from the phone.
 *
 * Author: Carson Katri <me@carsonkatri.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "status_bar_layer.h"
#include "property_animation.h"
#include "timeline.h"
#include "blob_db.h"

typedef struct NotificationLayer
{
    Layer layer;
    Layer content_layer;
    ScrollLayer scroll_layer;
    StatusBarLayer status_bar;
    Layer status_index_layer;
    ClickHandler back_click_handler;

    uint16_t offset;
    uint16_t anim_offset_start;
    uint16_t anim_offset_goal;
    
    PropertyAnimation prop_anim;
   
    uint8_t notif_count;
    uint8_t selected_notif_idx;

    /* Ui message */
    char *subject;
    char *sender;
    char *body;
    uint32_t message_type;

    rebble_notification *notification;

    ResultSetList all_notifications;
    ResultSetItem selected_result_item;

    /* some display properties */
    GColor type_color;
    GBitmap *icon;
} NotificationLayer;

static void notification_layer_ctor(NotificationLayer *notification_layer, GRect frame);
static void notification_layer_dtor(NotificationLayer *notification_layer);

NotificationLayer *notification_layer_create(GRect frame);
void notification_layer_destroy(NotificationLayer *notification_layer);

Layer* notification_layer_get_layer(NotificationLayer *notification_layer);
void notification_layer_configure_click_config(NotificationLayer *notification_layer, 
                                               Window *window,
                                               ClickHandler back_click_handler);

void notification_layer_message_arrived(NotificationLayer *notification_layer, Uuid *uuid);
