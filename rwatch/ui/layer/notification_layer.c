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
#include "blob_db.h"
#include "timeline.h"
#include "notification_manager.h"
#include "platform_res.h"

/* Configure Logging */
#define MODULE_NAME "notyla"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


static void _notification_layer_update_proc(Layer *layer, GContext *ctx);
static void _status_index_draw(Layer *layer, GContext *ctx);
static void _click_config_provider(void *context);


static void notification_layer_ctor(NotificationLayer *notification_layer, GRect frame)
{
    layer_ctor(&notification_layer->layer, frame);
    layer_ctor(&notification_layer->content_layer, frame);

    scroll_layer_ctor(&notification_layer->scroll_layer, GRect(0, 0, frame.size.w, 200));
    scroll_layer_add_child(&notification_layer->scroll_layer, &notification_layer->layer);
    layer_add_child(&notification_layer->content_layer, scroll_layer_get_layer(&notification_layer->scroll_layer));

    status_bar_layer_ctor(&notification_layer->status_bar);
    status_bar_layer_set_colors(&notification_layer->status_bar, GColorGreen, GColorBlack);
    layer_add_child(&notification_layer->content_layer, status_bar_layer_get_layer(&notification_layer->status_bar));

#ifdef PBL_RECT
    layer_ctor(&notification_layer->status_index_layer, notification_layer->status_bar.layer.frame);
    notification_layer->status_index_layer.callback_data = notification_layer;
    layer_set_update_proc(&notification_layer->status_index_layer, _status_index_draw);
    layer_add_child(status_bar_layer_get_layer(&notification_layer->status_bar), &notification_layer->status_index_layer);
#endif

    layer_set_update_proc(&notification_layer->layer, _notification_layer_update_proc);

    /* Load in the messages from the database.
     We actually only want a key value of each message 
     We are also fitering on message time for now */
    /* XXX TODO make this now - some time */
    uint32_t val_timestamp = 1550699327;
    ResultSetList lh = timeline_notifications(val_timestamp);

    notification_layer->all_notifications = lh;
    notification_layer->selected_result_item = blobdb_first_result(lh);

    uint8_t msg_count = 0;
    ResultSetItem rs;
    blobresult_foreach(rs, lh)
    {
        msg_count++;
    }
//     blobdb_resultset_destroy(lh);

    notification_layer->notif_count = msg_count;

    LOG_INFO("N CTOR %d", msg_count);
}

void _load_notification(NotificationLayer *notification_layer, Uuid *uuid)
{
    LOG_DEBUG("FREE 1: %d", app_heap_bytes_free());
    fonts_resetcache();
    LOG_DEBUG("FREE 2: %d", app_heap_bytes_free());

    if (notification_layer->notification)
        timeline_destroy(notification_layer->notification);

    if (notification_layer->icon)
    {
        gbitmap_destroy(notification_layer->icon);
        notification_layer->icon = NULL;
    }

    rebble_notification *notif = timeline_get_notification(uuid);
    
    LOG_DEBUG("FREE 3: %d", app_heap_bytes_free());

    if (!notif)
    {
        LOG_ERROR("Invalid Notification Id!");
        return;
    }

    notification_layer->notification = notif;
    notification_layer->sender = NULL;
    notification_layer->subject = NULL;
    notification_layer->body = NULL;
    notification_layer->message_type = 0;

    /* set subject */
    rebble_attribute *n;
    list_foreach(n, &notif->attributes, rebble_attribute, node)
    {
        if (n->timeline_attribute.length == 0)
            continue;

        switch (n->timeline_attribute.attribute_id)
        {
            case TimelineAttributeType_Sender:
                notification_layer->sender = (char *)n->data;
                break;
            case TimelineAttributeType_Subject:
                notification_layer->subject = (char *)n->data;
                break;
            case TimelineAttributeType_Message:
                notification_layer->body = (char *)n->data;
                break;
            case TimelineAttributeType_SourceType:
                notification_layer->message_type = *(uint32_t *)n->data;

                switch (notification_layer->message_type)
                {
                    case TimelineNotificationSource_SMS:
                        notification_layer->type_color = GColorGreen;
                        notification_layer->icon = gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE);
                        break;
                    default:
                        notification_layer->type_color = GColorBlueMoon;
                }
                break;
        }
    }
    
    
    LOG_DEBUG("FREE 4: %d", app_heap_bytes_free());
    window_dirty(true);
    scroll_layer_set_content_offset(&notification_layer->scroll_layer, GPoint(0, 0), true);
}

void notification_layer_message_arrived(NotificationLayer *notification_layer, Uuid *uuid)
{
    LOG_DEBUG("AR FREE: %d", app_heap_bytes_free());

    if (notification_layer->all_notifications)
    {
        blobdb_add_result_set_item(notification_layer->all_notifications, uuid, xTaskGetTickCount());
        notification_layer->selected_result_item = blobdb_first_result(notification_layer->all_notifications);
        notification_layer->notif_count++;
    }

    _load_notification(notification_layer, uuid);
}

static void notification_layer_dtor(NotificationLayer *notification_layer)
{
    /* delete message */
    timeline_destroy(notification_layer->notification);
    scroll_layer_dtor(&notification_layer->scroll_layer);
    layer_remove_from_parent(&notification_layer->status_bar.layer);
    status_bar_layer_dtor(&notification_layer->status_bar);
    layer_dtor(&notification_layer->layer);

#ifdef PBL_RECT
    layer_remove_from_parent(&notification_layer->status_index_layer);
    layer_dtor(&notification_layer->status_index_layer);
#endif

    /* clean up the ui */
    if (notification_layer->icon)
        gbitmap_destroy(notification_layer->icon);

    if (notification_layer->all_notifications)
        blobdb_resultset_destroy(notification_layer->all_notifications);

    LOG_DEBUG("FREE: %d", app_heap_bytes_free());
    LOG_INFO("N DTOR");
}

static void _status_index_draw(Layer *layer, GContext *ctx)
{
    NotificationLayer *notification_layer = layer->callback_data;

    // Draw the status bar indicator
    char status[10];
    snprintf(status, 10, "%d/%d", notification_layer->selected_notif_idx + 1, notification_layer->notif_count);
    ctx->text_color = status_bar_layer_get_foreground_color(&notification_layer->status_bar);
    GRect status_rect = GRect(0, -2, layer->bounds.size.w - 5, 14);
    graphics_draw_text(ctx, status, fonts_get_system_font(FONT_KEY_GOTHIC_14), status_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, 0);
}

NotificationLayer *notification_layer_create(GRect frame)
{
    LOG_INFO("Created Notification Layer");

    /* Make the window */
    NotificationLayer *notification_layer = app_calloc(1, sizeof(NotificationLayer));
    notification_layer_ctor(notification_layer, frame);

    return notification_layer;
}

void notification_layer_destroy(NotificationLayer *notification_layer)
{
    notification_layer_dtor(notification_layer);
    app_free(notification_layer);
    fonts_resetcache();
    LOG_DEBUG("FREE: %d", app_heap_bytes_free());
    window_dirty(true);
}

Layer* notification_layer_get_layer(NotificationLayer *notification_layer)
{
    return &notification_layer->content_layer;
}

void notification_layer_configure_click_config(NotificationLayer *notification_layer, 
                                               Window *window,
                                               ClickHandler back_click_handler)
{
    notification_layer->back_click_handler = back_click_handler;

    window_set_click_config_provider_with_context(window, 
                                                  (ClickConfigProvider)_click_config_provider, 
                                                  notification_layer);

    _click_config_provider(notification_layer);
}

static void _scroll_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;
    GPoint p = scroll_layer_get_content_offset(&notification_layer->scroll_layer);
    LOG_DEBUG("P x %d y %d", p.x, p.y);
    notification_reschedule_timer(layer_get_window(&notification_layer->content_layer), REARM_TIMEOUT_MS);

    if (p.y < 0)
    {
        scroll_layer_scroll_up_click_handler(recognizer, &notification_layer->scroll_layer);
        window_dirty(true);
        return;
    }

    if (notification_layer->selected_notif_idx <= 0)
        return;

    notification_layer->selected_notif_idx--;

    ResultSetItem ri = blobdb_prev_result(notification_layer->all_notifications, notification_layer->selected_result_item);
    if (!ri)
        return;

    notification_layer->selected_result_item = ri;

    _load_notification(notification_layer, (Uuid *)ri->select1);

    window_dirty(true);
    scroll_layer_set_content_offset(&notification_layer->scroll_layer, GPoint(0, 0), true);
}

static void _scroll_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;
    GPoint p = scroll_layer_get_content_offset(&notification_layer->scroll_layer);
    LOG_DEBUG("P x %d y %d", p.x, p.y);
    notification_reschedule_timer(layer_get_window(&notification_layer->content_layer), REARM_TIMEOUT_MS);

    if (p.y > -(200 - notification_layer->layer.frame.size.h))
    {
        scroll_layer_scroll_down_click_handler(recognizer, &notification_layer->scroll_layer);
        window_dirty(true);
        return;
    }

    if (notification_layer->selected_notif_idx >= notification_layer->notif_count)
        return;

    notification_layer->selected_notif_idx++;

    ResultSetItem ri = blobdb_next_result(notification_layer->all_notifications, notification_layer->selected_result_item);
    if (!ri)
        return;

    notification_layer->selected_result_item = ri;

    _load_notification(notification_layer, (Uuid *)ri->select1);

    window_dirty(true);
    scroll_layer_set_content_offset(&notification_layer->scroll_layer, GPoint(0, 0), true);
}

static void _action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
    LOG_DEBUG("AP Con %x", context);
    rebble_action *na = (rebble_action *)context;
    NotificationLayer *nl = (NotificationLayer *)action_menu_get_context(action_menu);
    timeline_action_send(na->timeline_action.action_id, &nl->notification->timeline_item.uuid, NULL, NULL);
}

static void _show_options_click_handler(ClickRecognizerRef recognizer, void *context)
{
    NotificationLayer *notification_layer = (NotificationLayer *)context;

    rebble_action *na;
    rebble_attribute *aa;
    int actions = 0;
    
    list_foreach(na, &notification_layer->notification->actions, rebble_action, node)
        actions++;
        
    ActionMenuLevel *root_level = action_menu_level_create(actions);
    
    list_foreach(na, &notification_layer->notification->actions, rebble_action, node)
    {
        LOG_DEBUG("ACTION Id %d type %d cnt %d", na->timeline_action.action_id, na->timeline_action.type, na->timeline_action.attr_count);
        
        /* each attribute */
        list_foreach(aa, &na->attributes, rebble_attribute, node)
        {
            LOG_DEBUG("- ATTR Id %d len %d data %s", aa->timeline_attribute.attribute_id, aa->timeline_attribute.length, aa->data);
            
            switch (aa->timeline_attribute.attribute_id)
            {
                case TimelineAttributeType_Sender:
                    action_menu_level_add_action(root_level, aa->data, _action_performed_callback, na);
                    break;
            }            
        }
    }
    
    // Make + show the ActionMenu
    /*ActionMenuLevel *root_level = action_menu_level_create(3);
    ActionMenuLevel *reply_level = action_menu_level_create(3);

    action_menu_level_add_child(root_level, reply_level, "Reply");
    action_menu_level_add_action(root_level, "Dismiss", _action_performed_callback, 1);
    action_menu_level_add_action(root_level, "Dismiss All", _action_performed_callback, 2);

    action_menu_level_add_action(reply_level, "Dictate", _action_performed_callback, 3);
    action_menu_level_add_action(reply_level, "Canned Response", _action_performed_callback, 4);
    action_menu_level_add_action(reply_level, "Emoji", _action_performed_callback, 5);
*/
    ActionMenuConfig config = (ActionMenuConfig)
    {
        .root_level = root_level,
        .context = notification_layer,
        .colors = {
            .background = GColorBlack,
            .foreground = GColorWhite,
        },
        .align = ActionMenuAlignCenter
    };

    action_menu_open(&config);
}

static void _click_config_provider(void *context)
{
    NotificationLayer *nl = (NotificationLayer *)context;
    assert(nl);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) _scroll_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) _scroll_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) _show_options_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler) nl->back_click_handler);

    window_set_click_context(BUTTON_ID_DOWN, context);
    window_set_click_context(BUTTON_ID_UP, context);
    window_set_click_context(BUTTON_ID_SELECT, context);
    window_set_click_context(BUTTON_ID_BACK, context);
}

static void _notification_layer_update_proc(Layer *layer, GContext *ctx)
{
    
    NotificationLayer *notification_layer = container_of(layer, NotificationLayer, layer);
    assert(notification_layer);

    int offset = -STATUS_BAR_LAYER_HEIGHT + notification_layer->offset; // offset for the status_bar
    GRect bounds = layer_get_unobstructed_bounds(layer);

    GBitmap *icon = notification_layer->icon;
    char *app = notification_layer->sender ? notification_layer->sender : "Unknown";
    char *title = notification_layer->subject ? notification_layer->subject : "N/A";
    char *body = (char *)notification_layer->body ? (char *)notification_layer->body : "N/A";

    // Draw the background:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, notification_layer->type_color);
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
//#ifdef PBL_RECT
    ctx->text_color = GColorBlack;
//#else
 //   ctx->text_color = notification->color;
//#endif
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
    /*if (notification->node.next != NULL && notification_layer->offset > 0)
    {
        Notification *n = list_elem(notification->node.next, Notification, node);
        graphics_context_set_fill_color(ctx, n->color);
        n_graphics_fill_circle(ctx, GPoint(DISPLAY_COLS / 2, DISPLAY_ROWS - 2), 10);
    }*/
#endif

//     NotificationLayer *notification_layer = (NotificationLayer *)context;
//    Notification *notification = notification_layer->active;


}
