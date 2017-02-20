/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "librebble.h"
#include "scroll_layer.h"
#include "text.h"

ScrollLayer *scroll_layer_create(GRect bounds)
{
    ScrollLayer* slayer = (ScrollLayer*)calloc(1, sizeof(ScrollLayer));
    Layer* layer = layer_create(GRect(0, 0, 144, 168));  // TODO: size of the actual window 
    Layer* sublayer = layer_create(bounds);
    // give the layer a reference back to us
    layer->container = slayer;
    slayer->layer = layer;
    slayer->content_sublayer = sublayer;

    return slayer;
}

void scroll_layer_destroy(ScrollLayer *layer)
{
    layer_destroy(layer->layer);
    free(layer);
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
    // TODO
}

void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks)
{
    scroll_layer->callbacks = callbacks;
}

void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context)
{
    layer_set_context(scroll_layer->layer, context);
}

void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated)
{
    GRect bounds = layer_get_bounds(scroll_layer->content_sublayer);
    layer_set_bounds(scroll_layer->content_sublayer, GRect(offset.x, offset.y, bounds.size.w, bounds.size.h));
    // TODO animate to new offset
}

GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer)
{
    GRect bounds = layer_get_bounds(scroll_layer->content_sublayer);
    return bounds.origin;
}

void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size)
{
    GRect bounds = layer_get_bounds(scroll_layer->content_sublayer);
    layer_set_bounds(scroll_layer->content_sublayer, GRect(bounds.origin.x, bounds.origin.y, size.w, size.h));
}

GSize scroll_layer_get_content_size(const ScrollLayer *scroll_layer)
{
    GRect bounds = layer_get_bounds(scroll_layer->content_sublayer);
    return bounds.size;
}

void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame)
{
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
