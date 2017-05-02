/* text_layer.c
 * routines for [...]
 * RebbleOS core
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "text.h"

void text_layer_draw(struct Layer *layer, GContext *context);

// Layer Functions
TextLayer *text_layer_create(GRect bounds)
{
    TextLayer* tlayer = (TextLayer*)calloc(1, sizeof(TextLayer));
    Layer* layer = layer_create(bounds);
    // give the layer a reference back to us
    layer->container = tlayer;
    tlayer->layer = layer;
    
    // hook the draw callback to us
    // this way we control the text, bound, pagination etc
    layer_set_update_proc(layer, text_layer_draw);
    return tlayer;
}

void text_layer_destroy(TextLayer *layer)
{
    layer_destroy(layer->layer);
    free(layer);
}

Layer *text_layer_get_layer(TextLayer *text_layer)
{
    return text_layer->layer;
}

void text_layer_set_text(TextLayer *text_layer, const char* text)
{
    text_layer->text = text;
}

const char *text_layer_get_text(TextLayer *text_layer)
{
    return text_layer->text;
}

void text_layer_set_background_color(TextLayer *text_layer, GColor color)
{
    text_layer->background_color = color;
}

void text_layer_set_text_color(TextLayer *text_layer, GColor color)
{
    text_layer->text_color = color;
}

void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode)
{
    text_layer->overflow_mode = line_mode;
}

void text_layer_set_font(TextLayer * text_layer, GFont font)
{
    text_layer->font = font;
}

void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment)
{
    text_layer->text_alignment = text_alignment;
}

GSize text_layer_get_content_size(TextLayer *text_layer)
{
    return text_layer->layer->bounds.size;
}

void text_layer_set_size(TextLayer *text_layer, const GSize max_size)
{
    text_layer->layer->bounds.size.w = max_size.w;
    text_layer->layer->bounds.size.h = max_size.h;
}

void text_layer_draw(struct Layer *layer, GContext *context)
{
    TextLayer *tlayer = (TextLayer *)layer->container;
    printf("cb\n");
    // we are goingto draw the text now
    graphics_draw_text(context, tlayer->text, tlayer->font,
                       layer->bounds, tlayer->overflow_mode,
                       tlayer->text_alignment, &tlayer->text_attributes);
}

// TODO paging...

void text_layer_enable_screen_text_flow_and_paging(TextLayer *text_layer, uint8_t inset)
{
}

void text_layer_restore_default_text_flow_and_paging(TextLayer *text_layer)
{
}
