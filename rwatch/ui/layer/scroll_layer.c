/* scroll_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "scroll_layer.h"
#include "utils.h"
#include "property_animation.h"


/* Configure Logging */
#define MODULE_NAME "slayer"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


#define BUTTON_REPEAT_INTERVAL_MS 600
#define CLICK_SCROLL_AMOUNT 16
#define ANIMATE_ON_CLICK true

void scroll_layer_ctor(ScrollLayer* slayer, GRect frame)
{
    layer_ctor(&slayer->layer, frame);
    layer_ctor(&slayer->content_sublayer, frame);

    // give the layer a reference back to us
    slayer->layer.container = slayer;
    slayer->context = slayer;

    layer_add_child(&slayer->layer, &slayer->content_sublayer);
}

void scroll_layer_dtor(ScrollLayer* slayer)
{
    layer_dtor(&slayer->layer);
    layer_dtor(&slayer->content_sublayer);
}

ScrollLayer *scroll_layer_create(GRect frame)
{
    ScrollLayer* slayer = app_calloc(1, sizeof(ScrollLayer));
    scroll_layer_ctor(slayer, frame);

    return slayer;
}

void scroll_layer_destroy(ScrollLayer *layer)
{
    scroll_layer_dtor(layer);
    app_free(layer);
}

static void _scroll_layer_add_content_offset(ScrollLayer *layer, GPoint offset, bool animate) {
    GPoint current = layer_get_frame(&layer->content_sublayer).origin;
    offset.x += current.x;
    offset.y += current.y;

    scroll_layer_set_content_offset(layer, offset, animate);
}

static void _down_single_click_handler(ClickRecognizerRef _, void *layer)
{
    ScrollLayer *slayer = (ScrollLayer *)layer;
    int scroll_amount = slayer->paging_enabled ? -DISPLAY_ROWS : -CLICK_SCROLL_AMOUNT;

    _scroll_layer_add_content_offset(slayer, GPoint(0, scroll_amount), ANIMATE_ON_CLICK);
}

static void _up_single_click_handler(ClickRecognizerRef _, void *layer)
{
    ScrollLayer *slayer = (ScrollLayer *)layer;
    int scroll_amount = slayer->paging_enabled ? DISPLAY_ROWS : CLICK_SCROLL_AMOUNT;
    _scroll_layer_add_content_offset(slayer, GPoint(0, scroll_amount), ANIMATE_ON_CLICK);
}

static void _scroll_layer_click_config_provider(ScrollLayer *layer)
{
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_REPEAT_INTERVAL_MS, _down_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_REPEAT_INTERVAL_MS, _up_single_click_handler);
    window_set_click_context(BUTTON_ID_DOWN, layer);
    window_set_click_context(BUTTON_ID_UP, layer);

    if (layer->callbacks.click_config_provider) {
       layer->callbacks.click_config_provider(layer->context);
       window_set_click_context(BUTTON_ID_SELECT, layer->context);
    }
}

Layer *scroll_layer_get_layer(const ScrollLayer *scroll_layer)
{
    return (Layer *)&scroll_layer->layer;
}

void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child)
{
    layer_add_child(&scroll_layer->content_sublayer, child);
}

void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window)
{
    window_set_click_config_provider_with_context(window, (ClickConfigProvider)_scroll_layer_click_config_provider, scroll_layer);
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
    GSize slayer_size = layer_get_frame(&scroll_layer->layer).size;
    GRect frame = layer_get_frame(&scroll_layer->content_sublayer);

    scroll_layer->prev_scroll_offset = scroll_layer->scroll_offset;
    scroll_layer->scroll_offset = GRect(CLAMP(offset.x, -frame.size.w, slayer_size.w),
                                 CLAMP(offset.y, -frame.size.h, slayer_size.h),
                                 frame.size.w,
                                 frame.size.h);

    if (animated)
    {
        if (scroll_layer->animation)
        {
            Animation *panim = property_animation_get_animation(scroll_layer->animation);
//             if (panim->onqueue)
// //             if (appmanager_timer_remove(panim))
//             {
// //                 property_animation_destroy(scroll_layer->animation);
// //                 appmanager_timer_remove(&panim->timer);
//                 LOG_INFO("[%x] UNSCHED", panim);
//                 animation_unschedule(panim);
//             }
        }

        scroll_layer->animation = property_animation_create_layer_frame(&scroll_layer->content_sublayer, 
                                                                    &scroll_layer->prev_scroll_offset, 
                                                                    &scroll_layer->scroll_offset);
        Animation *anim = property_animation_get_animation(scroll_layer->animation);
        animation_set_duration(anim, 100);
        animation_schedule(anim);
    }
    else
    {
        layer_set_frame(&scroll_layer->content_sublayer, scroll_layer->scroll_offset);
    }
}

GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer)
{
    GRect frame = layer_get_frame(&scroll_layer->content_sublayer);
    return frame.origin;
}

void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size)
{
    GRect frame = layer_get_frame(&scroll_layer->content_sublayer);
    frame.size = size;
    layer_set_frame(&scroll_layer->content_sublayer, frame);
    scroll_layer->scroll_offset = scroll_layer->prev_scroll_offset = frame;
    // calling set_offset to ensure that offset is clamped to current size
    scroll_layer_set_content_offset(scroll_layer, frame.origin, false);
}

GSize scroll_layer_get_content_size(const ScrollLayer *scroll_layer)
{
    GRect bounds = layer_get_frame(&scroll_layer->content_sublayer);
    return bounds.size;
}

void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame)
{
    layer_set_frame(&scroll_layer->layer, frame);

    // clamp content offset to new size
    GPoint offset = layer_get_frame(&scroll_layer->content_sublayer).origin;
    scroll_layer_set_content_offset(scroll_layer, offset, false);
}

void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    _up_single_click_handler(recognizer, context);
}

void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    _down_single_click_handler(recognizer, context);
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
    scroll_layer->paging_enabled = paging_enabled;
}

bool scroll_layer_get_paging(ScrollLayer *scroll_layer)
{
    return scroll_layer->paging_enabled;
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

bool content_indicator_configure_direction(ContentIndicator *content_indicator, 
                                           ContentIndicatorDirection direction, 
                                           const ContentIndicatorConfig *config)
{
    return true;
}

bool content_indicator_get_content_available(ContentIndicator *content_indicator, 
                                             ContentIndicatorDirection direction)
{
    return true;
}

void content_indicator_set_content_available(ContentIndicator *content_indicator, 
                                             ContentIndicatorDirection direction, 
                                             bool available)
{
}
