/* single_notification_layer.c
 * Renders a single notification.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 *
 * based off of a design by @lavender
 *
 * Renders *just* the notification itself.  If you want to scroll it, you
 * need to put it in a scroll-layer yourself.
 */

#include "librebble.h"
#include "ngfxwrap.h"
#include "notification_manager.h"
#include "platform_res.h"
#include "single_notification_layer.h"
#include "minilib.h"

#include "status_bar_layer.h"

static void single_notification_layer_update_proc(Layer *layer, GContext *ctx);

#define X_PADDING 4

#define APPNAME_HEIGHT 28
#define APPNAME_PADDING 8
#define ELEMENT_PADDING 8
#define APPNAME_FONT FONT_KEY_GOTHIC_24_BOLD
#define TITLE_FONT FONT_KEY_GOTHIC_18_BOLD
#define SUBTITLE_FONT FONT_KEY_GOTHIC_18
#define BODY_FONT FONT_KEY_GOTHIC_24_BOLD
#define TIMESTAMP_FONT FONT_KEY_GOTHIC_14

void single_notification_layer_ctor(SingleNotificationLayer *l, GRect frame)
{
    layer_ctor(&l->layer, frame);
    layer_set_update_proc(&l->layer, single_notification_layer_update_proc);

    l->title = l->subtitle = l->body = l->timestamp = NULL;
    l->icon = NULL;
}

void single_notification_layer_dtor(SingleNotificationLayer *l)
{
    free(l->title);
    free(l->subtitle);
    free(l->body);
    free(l->timestamp);
    gbitmap_destroy(l->icon);
    layer_dtor(&l->layer);
}

extern void single_notification_layer_set_notification(SingleNotificationLayer *l, rebble_notification *notif)
{
    free(l->title);
    free(l->subtitle);
    free(l->body);
    free(l->timestamp);
    if (l->icon)
        gbitmap_destroy(l->icon);

    const char *sender = NULL, *subject = NULL, *message = NULL;
    uint32_t sourcetype = 0;

    rebble_attribute *a;
    list_foreach(a, &notif->attributes, rebble_attribute, node)
    {
        switch (a->timeline_attribute.attribute_id)
        {
        case TimelineAttributeType_Sender:
            sender = (const char *)a->data;
            break;
        case TimelineAttributeType_Subject:
            subject = (const char *)a->data;
            break;
        case TimelineAttributeType_Message:
            message = (const char *)a->data;
            break;
        case TimelineAttributeType_SourceType:
            sourcetype = *(uint32_t *)a->data;
            break;
        default:
            /* we don't care */
            ;
        }
    }

    if (sender && strlen(sender))
    {
        l->title = strdup(sender);
        if (subject && strlen(subject))
            l->subtitle = strdup(subject);
        l->body = message ? strdup(message) : NULL;
    }
    else
    {
        if (subject && strlen(subject))
            l->title = strdup(subject);
        l->body = message ? strdup(message) : NULL;
    }

    switch (sourcetype)
    {
        // since the 'source' will be useful just to set its respective icon, there's no need
        // to hard code the app name using it. 'subject' will be used instead.
    case TimelineNotificationSource_SMS:
        //l->source = "SMS";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_NOTIFICATION);
        break;
    case TimelineNotificationSource_Email:
        //l->source = "Email";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_NOTIFICATION);
        break;
    case TimelineNotificationSource_Twitter:
        //l->source = "Twitter";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
        break;
    case TimelineNotificationSource_Facebook:
        //l->source = "Facebook";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_NOTIFICATION);
        break;
    default:
        l->source = NULL;
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
        break;
    }

    time_t now = rcore_get_time();
    time_t ts = notif->timeline_item.timestamp;
    struct tm tm_ts;
    localtime_r(&ts, &tm_ts);

    char buf[32];

    if (ts > now)
        l->timestamp = strdup("The future");
    else if (ts > (now - 60 * 60))
    {
        sfmt(buf, sizeof(buf), "%d min ago", (ts - now) / 60);
        l->timestamp = strdup(buf);
    }
    else if (ts > (now - 24 * 60 * 60))
    {
        sfmt(buf, sizeof(buf), "%d hours ago", (ts - now) / (60 * 60));
        l->timestamp = strdup(buf);
    }
    else if (ts > (now - 7 * 24 * 60 * 60))
    {
        char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        sfmt(buf, sizeof(buf), "%s, %02d:%02d", days[tm_ts.tm_mday], tm_ts.tm_hour, tm_ts.tm_min);
        l->timestamp = strdup(buf);
    }
    else
    {
        char *mons[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        sfmt(buf, sizeof(buf), "%s %d, %02d:%02d", mons[tm_ts.tm_mon], tm_ts.tm_mday, tm_ts.tm_hour, tm_ts.tm_min);
        l->timestamp = strdup(buf);
    }

    layer_mark_dirty(&l->layer);
}

Layer *single_notification_layer_get_layer(SingleNotificationLayer *l)
{
    return &l->layer;
}

//#ifdef PBL_RECT

uint16_t single_notification_layer_height(SingleNotificationLayer *l)
{
    uint16_t height = 0;

    GRect szrect = layer_get_frame(&l->layer);
    szrect.size.h = 1000;
    szrect.size.w -= X_PADDING * 2;

    height += APPNAME_HEIGHT;
    height += APPNAME_PADDING;
    if (l->title)
        height += n_graphics_text_layout_get_content_size_with_attributes(
                      l->title, fonts_get_system_font(TITLE_FONT),
                      szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0)
                      .h +
                  ELEMENT_PADDING;
    if (l->subtitle)
        height += n_graphics_text_layout_get_content_size_with_attributes(
                      l->subtitle, fonts_get_system_font(SUBTITLE_FONT),
                      szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0)
                      .h +
                  ELEMENT_PADDING;
    if (l->body)
        height += n_graphics_text_layout_get_content_size_with_attributes(
                      l->body, fonts_get_system_font(BODY_FONT),
                      szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0)
                      .h +
                  ELEMENT_PADDING;
    height += n_graphics_text_layout_get_content_size_with_attributes(
                  l->timestamp, fonts_get_system_font(TIMESTAMP_FONT),
                  szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0)
                  .h +
              ELEMENT_PADDING;

    return height;
}

static void single_notification_layer_update_proc(Layer *layer, GContext *ctx)
{
    SingleNotificationLayer *l = container_of(layer, SingleNotificationLayer, layer);
    GRect szrect = layer_get_frame(layer);
    GSize outsz;

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, szrect, 0, GCornerNone);

    graphics_context_set_text_color(ctx, GColorBlack);

    szrect.origin.x = 0;
    szrect.origin.y = STATUS_BAR_LAYER_HEIGHT;

    szrect.origin.x += X_PADDING;
    szrect.size.w -= X_PADDING * 2;

    /* 
from libpebble2.services.notifications import Notifications
Notifications(pebble, None).send_notification(subject = "hello", sender = "How Are you?")
    */

    GRect tmpsz = szrect;
    tmpsz.size.h = 25;
    tmpsz.size.w = 25;
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, l->icon, tmpsz);

    tmpsz = szrect;
    tmpsz.size.h = APPNAME_HEIGHT; // Align text vertically
    tmpsz.size.w -= APPNAME_HEIGHT + ELEMENT_PADDING;
    tmpsz.origin.x += APPNAME_HEIGHT;

    if (l->title)
    {
        graphics_draw_text(ctx, l->title, fonts_get_system_font(APPNAME_FONT),
                           tmpsz, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
        szrect.origin.y += APPNAME_HEIGHT;
        szrect.size.h -= APPNAME_HEIGHT;
    }

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);

    /* XXX: make this a drawrect */
    graphics_draw_line(ctx,
                       GPoint(szrect.origin.x, szrect.origin.y),
                       GPoint(szrect.origin.x + szrect.size.w, szrect.origin.y));
    szrect.origin.y += 0;
    szrect.size.h -= APPNAME_PADDING;

    if (l->subtitle)
    {
        graphics_draw_text_ex(ctx,
                              l->subtitle, fonts_get_system_font(SUBTITLE_FONT),
                              szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
        szrect.origin.y += outsz.h;
        szrect.size.h -= outsz.h;
    }
    if (l->body)
    {
        graphics_draw_text_ex(ctx,
                              l->body, fonts_get_system_font(BODY_FONT),
                              szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
        szrect.origin.y += outsz.h + ELEMENT_PADDING;
        szrect.size.h -= outsz.h + ELEMENT_PADDING;
    }
    graphics_draw_text_ex(ctx,
                          l->timestamp, fonts_get_system_font(TIMESTAMP_FONT),
                          szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
    szrect.origin.y += outsz.h + ELEMENT_PADDING;
    szrect.size.h -= outsz.h + ELEMENT_PADDING;
}

//#else /* !PBL_RECT */
//#error single_notification_layer not implemented on non-rectangular Pebbles
//#endif
