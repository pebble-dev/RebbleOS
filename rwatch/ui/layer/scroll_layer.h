#pragma once
/* scrol_layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

//#include "librebble.h"
#include "point.h"
#include "rect.h"
#include "size.h"
#include "animation.h"
#include "click_config.h"
#include "content_indicator.h"

struct ScrollLayer;

typedef void (*ScrollLayerCallback)(struct ScrollLayer *scroll_layer, void *context);
typedef struct ScrollLayerCallbacks
{
    ClickConfigProvider click_config_provider;
    ScrollLayerCallback content_offset_changed_handler;
} ScrollLayerCallbacks;

typedef struct ScrollLayer
{
    Layer layer;
    Layer content_sublayer;
//     Layer shadow_sublayer;
    PropertyAnimation *animation;
    GRect prev_scroll_offset;
    GRect scroll_offset;
    bool paging_enabled;
    ScrollLayerCallbacks callbacks;
    void *context;
} ScrollLayer;

void scroll_layer_ctor(ScrollLayer* slayer, GRect frame);
void scroll_layer_dtor(ScrollLayer* slayer);

ScrollLayer *scroll_layer_create(GRect frame);
void scroll_layer_destroy(ScrollLayer *layer);
Layer *scroll_layer_get_layer(const ScrollLayer *scroll_layer);

void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child);
void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window);
void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks);
void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context);
void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated);
GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer);
void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size);
GSize scroll_layer_get_content_size(const ScrollLayer *scroll_layer);
void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame);
void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, void *context);
void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, void *context);
void scroll_layer_set_shadow_hidden(ScrollLayer *scroll_layer, bool hidden);
bool scroll_layer_get_shadow_hidden(const ScrollLayer *scroll_layer);
void scroll_layer_set_paging(ScrollLayer *scroll_layer, bool paging_enabled);
bool scroll_layer_get_paging(ScrollLayer *scroll_layer);
ContentIndicator *scroll_layer_get_content_indicator(ScrollLayer *scroll_layer);
ContentIndicator *content_indicator_create(void);
void content_indicator_destroy(ContentIndicator *content_indicator);
bool content_indicator_configure_direction(ContentIndicator *content_indicator, ContentIndicatorDirection direction, const ContentIndicatorConfig *config);
bool content_indicator_get_content_available(ContentIndicator *content_indicator, ContentIndicatorDirection direction);
void content_indicator_set_content_available(ContentIndicator *content_indicator, ContentIndicatorDirection direction, bool available);
