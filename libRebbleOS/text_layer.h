#pragma once
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

typedef struct TextLayer
{
    Layer *layer;
    const char *text;
    GFont font;
//     GTextLayoutCacheRef layout_cache;
    GColor text_color;
    GColor background_color;
    GTextOverflowMode overflow_mode;
    GTextAlignment text_alignment;
    GTextAttributes text_attributes;
//     bool should_cache_layout;
} TextLayer;

TextLayer *text_layer_create(GRect frame);
void text_layer_destroy(TextLayer *text_layer);
Layer *text_layer_get_layer(TextLayer *text_layer);
void text_layer_set_text(TextLayer *text_layer, const char* text);
const char *text_layer_get_text(TextLayer *text_layer);
void text_layer_set_background_color(TextLayer *text_layer, GColor color);
void text_layer_set_text_color(TextLayer *text_layer, GColor color);
void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode);
void text_layer_set_font(TextLayer * text_layer, GFont font);
void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment);
void text_layer_enable_screen_text_flow_and_paging(TextLayer *text_layer, uint8_t inset);
void text_layer_restore_default_text_flow_and_paging(TextLayer *text_layer);
GSize text_layer_get_content_size(TextLayer *text_layer);
void text_layer_set_size(TextLayer *text_layer, const GSize max_size);
