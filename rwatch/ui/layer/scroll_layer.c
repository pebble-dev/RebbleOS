/* scroll_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "scroll_layer.h"
#include "utils.h"

#define BUTTON_REPEAT_INTERVAL_MS 600
#define CLICK_SCROLL_AMOUNT 16
#define ANIMATE_ON_CLICK true

ScrollLayer *scroll_layer_create(GRect frame)
{
    ScrollLayer* slayer = (ScrollLayer*)app_calloc(1, sizeof(ScrollLayer));
    Layer* layer = layer_create(frame);
    Layer* sublayer = layer_create(frame);
    // give the layer a reference back to us
    layer->container = slayer;
    slayer->layer = layer;
    slayer->content_sublayer = sublayer;
    slayer->context = slayer;

    layer_add_child(layer, sublayer);

    return slayer;
}

void scroll_layer_destroy(ScrollLayer *layer)
{
    layer_destroy(layer->layer);
    app_free(layer);
}

static void scroll_layer_add_content_offset(ScrollLayer *layer, GPoint offset, bool animate) {
    GPoint current = layer_get_frame(layer->content_sublayer).origin;
    offset.x += current.x;
    offset.y += current.y;

    scroll_layer_set_content_offset(layer, offset, animate);
}

static void down_single_click_handler(ClickRecognizerRef _, void *context)
{
    scroll_layer_add_content_offset(context, GPoint(0, -CLICK_SCROLL_AMOUNT), ANIMATE_ON_CLICK);
}

static void up_single_click_handler(ClickRecognizerRef _, void *context)
{
    scroll_layer_add_content_offset(context, GPoint(0, CLICK_SCROLL_AMOUNT), ANIMATE_ON_CLICK);
}


static void scroll_layer_click_config_provider(ScrollLayer *layer)
{
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_REPEAT_INTERVAL_MS, down_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_REPEAT_INTERVAL_MS, up_single_click_handler);
    window_set_click_context(BUTTON_ID_DOWN, layer);
    window_set_click_context(BUTTON_ID_UP, layer);

    if (layer->callbacks.click_config_provider) {
       layer->callbacks.click_config_provider(layer->context);
       window_set_click_context(BUTTON_ID_SELECT, layer->context);
    }
}

Layer *scroll_layer_get_layer(ScrollLayer *scroll_layer)
{
    return scroll_layer->layer;
}

void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child)
{
    layer_add_child(scroll_layer->content_sublayer, child);
}

void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window)
{
    window_set_click_config_provider_with_context(window, (ClickConfigProvider)scroll_layer_click_config_provider, scroll_layer);
}

void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks)
{
    scroll_layer->callbacks = callbacks;
}

void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context)
{
    scroll_layer->context = context ? context : scroll_layer;
}

void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated)
{
    GSize slayer_size = layer_get_frame(scroll_layer->layer).size;
    GRect frame = layer_get_frame(scroll_layer->content_sublayer);
    
    scroll_layer->scroll_offset = GRect(CLAMP(offset.x, slayer_size.w - frame.size.w, 0),
                                 CLAMP(offset.y, slayer_size.h - frame.size.h, 0),
                                 frame.size.w,
                                 frame.size.h);
    
    scroll_layer->animation = property_animation_create_layer_frame(scroll_layer->content_sublayer, &frame, &scroll_layer->scroll_offset);
    Animation *anim = property_animation_get_animation(scroll_layer->animation);
    animation_set_duration(anim, 100);
    animation_schedule(anim);
}

GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer)
{
    GRect frame = layer_get_frame(scroll_layer->content_sublayer);
    return frame.origin;
}

void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size)
{
    GRect frame = layer_get_frame(scroll_layer->content_sublayer);
    layer_set_frame(scroll_layer->content_sublayer, GRect(frame.origin.x, frame.origin.y, size.w, size.h));
    // calling set_offset to ensure that offset is clamped to current size
    scroll_layer_set_content_offset(scroll_layer, frame.origin, false);
}

GSize scroll_layer_get_content_size(const ScrollLayer *scroll_layer)
{
    GRect bounds = layer_get_frame(scroll_layer->content_sublayer);
    return bounds.size;
}

void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame)
{
    layer_set_frame(scroll_layer->layer, frame);

    // clamp content offset to new size
    GPoint offset = layer_get_frame(scroll_layer->content_sublayer).origin;
    scroll_layer_set_content_offset(scroll_layer, offset, false);
}

void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
}

void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
}

void scroll_layer_set_shadow_hidden(ScrollLayer *scroll_layer, bool hidden)
{
}

bool scroll_layer_get_shadow_hidden(const ScrollLayer *scroll_layer)
{
    return true;
}

void scroll_layer_set_paging(ScrollLayer *scroll_layer, bool paging_enabled)
{
}

bool scroll_layer_get_paging(ScrollLayer *scroll_layer)
{
    return true;
}

ContentIndicator *scroll_layer_get_content_indicator(ScrollLayer *scroll_layer)
{
    return NULL;
}

ContentIndicator *content_indicator_create(void)
{
    return NULL;
}


void content_indicator_destroy(ContentIndicator *content_indicator)
{
}

bool content_indicator_configure_direction(ContentIndicator *content_indicator, ContentIndicatorDirection direction, const ContentIndicatorConfig *config)
{
    return true;
}

bool content_indicator_get_content_available(ContentIndicator *content_indicator, ContentIndicatorDirection direction)
{
    return true;
}

void content_indicator_set_content_available(ContentIndicator *content_indicator, ContentIndicatorDirection direction, bool available)
{
}
