#pragma once
/* text_layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"

typedef struct TextLayer
{
    Layer layer;
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

void text_layer_dtor(TextLayer *tlayer);
void text_layer_ctor(TextLayer *tlayer, GRect frame);

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

TextLayer *text_layer_legacy2_create(GRect frame);
void text_layer_legacy2_destroy(TextLayer *layer);
GSize text_layer_legacy2_get_content_size(TextLayer *text_layer);
Layer *text_layer_legacy2_get_layer(TextLayer *text_layer);
const char *text_layer_legacy2_get_text(TextLayer *text_layer);
void text_layer_legacy2_set_background_color_2bit(TextLayer *text_layer, int color_2bit);
void text_layer_legacy2_set_font(TextLayer *text_layer, GFont font);
void text_layer_legacy2_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode);
void text_layer_legacy2_set_size(TextLayer *text_layer, const GSize max_size);
void text_layer_legacy2_set_text(TextLayer *text_layer, const char *text);
void text_layer_legacy2_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment);
void text_layer_legacy2_set_text_color_2bit(TextLayer *text_layer, int color_2bit);
