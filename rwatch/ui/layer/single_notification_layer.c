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

static void single_notification_layer_update_proc(Layer *layer, GContext *ctx);

#define X_PADDING 4

#define APPNAME_HEIGHT 28
#define APPNAME_PADDING 6
#define ELEMENT_PADDING 6
#define APPNAME_FONT   FONT_KEY_GOTHIC_28_BOLD
#define TITLE_FONT     FONT_KEY_GOTHIC_18_BOLD
#define SUBTITLE_FONT  FONT_KEY_GOTHIC_18
#define BODY_FONT      FONT_KEY_GOTHIC_24_BOLD
#define TIMESTAMP_FONT FONT_KEY_GOTHIC_14

void single_notification_layer_ctor(SingleNotificationLayer *l, rebble_notification *notif, GRect frame) {
    layer_ctor(&l->layer, frame);
    layer_set_update_proc(&l->layer, single_notification_layer_update_proc);
    
    const char *sender = NULL, *subject = NULL, *message = NULL;
    uint32_t sourcetype = 0;
    
    rebble_attribute *a;
    list_foreach(a, &notif->attributes, rebble_attribute, node) {
        switch (a->timeline_attribute.attribute_id) {
        case TimelineAttributeType_Sender:  sender  = (const char *) a->data; break;
        case TimelineAttributeType_Subject: subject = (const char *) a->data; break;
        case TimelineAttributeType_Message: message = (const char *) a->data; break;
        case TimelineAttributeType_SourceType:
            sourcetype = *(uint32_t *)a->data;
            break;
        default:
            /* we don't care */
            ;
        }
    }
    
    if (sender && strlen(sender)) {
        l->title = strdup(sender);
        if (subject && strlen(subject))
            l->subtitle = strdup(subject);
        l->body = message ? strdup(message) : NULL;
    } else {
        if (subject && strlen(subject))
            l->title = strdup(subject);
        l->body = message ? strdup(message) : NULL;
    }
    
    switch (sourcetype) {
    case TimelineNotificationSource_SMS:
        l->source = "SMS";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE);
        break;
    default:
        l->source = "Unknown";
        l->icon = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
        break;
    }
    
    /* notif->timestamp */
    l->timestamp = strdup("Just now");
}

void single_notification_layer_dtor(SingleNotificationLayer *l) {
    free(l->title);
    free(l->subtitle);
    free(l->body);
    free(l->timestamp);
    gbitmap_destroy(l->icon);
    layer_dtor(&l->layer);
}

Layer *single_notification_layer_get_layer(SingleNotificationLayer *l) {
    return &l->layer;
}

#ifdef PBL_RECT

uint16_t single_notification_layer_height(SingleNotificationLayer *l) {
    uint16_t height = 0;
    
    GRect szrect = layer_get_frame(&l->layer);
    szrect.size.h = 1000;
    szrect.size.w -= X_PADDING * 2;
    
    height += APPNAME_HEIGHT;
    height += APPNAME_PADDING;
    if (l->title)
        height += n_graphics_text_layout_get_content_size_with_attributes(
            l->title, fonts_get_system_font(TITLE_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0).h
            + ELEMENT_PADDING;
    if (l->subtitle)
        height += n_graphics_text_layout_get_content_size_with_attributes(
            l->subtitle, fonts_get_system_font(SUBTITLE_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0).h
            + ELEMENT_PADDING;
    if (l->body)
        height += n_graphics_text_layout_get_content_size_with_attributes(
            l->body, fonts_get_system_font(BODY_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0).h
            + ELEMENT_PADDING;
    height += n_graphics_text_layout_get_content_size_with_attributes(
        l->timestamp, fonts_get_system_font(TIMESTAMP_FONT),
        szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0).h
        + ELEMENT_PADDING;
    
    return height;
}

static void single_notification_layer_update_proc(Layer *layer, GContext *ctx) {
    SingleNotificationLayer *l = container_of(layer, SingleNotificationLayer, layer);
    GRect szrect = layer_get_frame(layer);
    GSize outsz;
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, szrect, 0, GCornerNone);
    
    graphics_context_set_text_color(ctx, GColorBlack);
    
    szrect.origin.x += X_PADDING;
    szrect.size.w   -= X_PADDING * 2;
    
    GRect tmpsz = szrect;
    tmpsz.origin.x += 3;
    tmpsz.origin.y += 3;
    tmpsz.size.h = APPNAME_HEIGHT - 6;
    tmpsz.size.w = APPNAME_HEIGHT - 6;
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, l->icon, tmpsz);
    
    tmpsz = szrect;
    tmpsz.size.h = APPNAME_HEIGHT;
    tmpsz.size.w   -= APPNAME_HEIGHT + ELEMENT_PADDING;
    tmpsz.origin.x += APPNAME_HEIGHT + ELEMENT_PADDING;
    n_graphics_draw_text(ctx, l->source, fonts_get_system_font(APPNAME_FONT),
        tmpsz, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0);
    szrect.origin.y += APPNAME_HEIGHT;
    szrect.size.h   -= APPNAME_HEIGHT;
    
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx,
        GPoint(szrect.origin.x, szrect.origin.y + 5),
        GPoint(szrect.origin.x + szrect.size.w, szrect.origin.y + 5));
    szrect.origin.y += APPNAME_PADDING;
    szrect.size.h   -= APPNAME_PADDING;
    
    if (l->title) {
        n_graphics_draw_text_ex(ctx,
            l->title, fonts_get_system_font(TITLE_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
        szrect.origin.y += outsz.h + ELEMENT_PADDING;
        szrect.size.h   -= outsz.h + ELEMENT_PADDING;
    }
    if (l->subtitle) {
        n_graphics_draw_text_ex(ctx,
            l->subtitle, fonts_get_system_font(SUBTITLE_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
        szrect.origin.y += outsz.h + ELEMENT_PADDING;
        szrect.size.h   -= outsz.h + ELEMENT_PADDING;
    }
    if (l->body) {
        n_graphics_draw_text_ex(ctx,
            l->body, fonts_get_system_font(BODY_FONT),
            szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
        szrect.origin.y += outsz.h + ELEMENT_PADDING;
        szrect.size.h   -= outsz.h + ELEMENT_PADDING;
        APP_LOG("noty", APP_LOG_LEVEL_INFO, "body outsz %d", outsz.h);
    }
    n_graphics_draw_text_ex(ctx,
        l->timestamp, fonts_get_system_font(TIMESTAMP_FONT),
        szrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, 0, &outsz);
    szrect.origin.y += outsz.h + ELEMENT_PADDING;
    szrect.size.h   -= outsz.h + ELEMENT_PADDING;
}

#else /* !PBL_RECT */
#   error single_notification_layer not implemented on non-rectangular Pebbles
#endif
