/* text_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "text.h"

void text_layer_draw(struct Layer *layer, GContext *context);

void text_layer_ctor(TextLayer *tlayer, GRect frame)
{
    layer_ctor(&tlayer->layer, frame);
    // give the layer a reference back to us
    tlayer->layer.container = tlayer;
    tlayer->text_color = GColorBlack;
    tlayer->background_color = GColorWhite;
    tlayer->text_alignment = GTextAlignmentLeft;
    tlayer->font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    tlayer->text = "";

    // hook the draw callback to us
    // this way we control the text, bound, pagination etc
    layer_set_update_proc(&tlayer->layer, text_layer_draw);
}

void text_layer_dtor(TextLayer *tlayer)
{
    layer_dtor(&tlayer->layer);
}

// Layer Functions
TextLayer *text_layer_create(GRect frame)
{
    TextLayer* tlayer = app_calloc(1, sizeof(TextLayer));
    text_layer_ctor(tlayer, frame);
    
    return tlayer;
}


void text_layer_destroy(TextLayer *layer)
{
    text_layer_dtor(layer);
    app_free(layer);
}

Layer *text_layer_get_layer(TextLayer *text_layer)
{
    return &text_layer->layer;
}

void text_layer_set_text(TextLayer *text_layer, const char* text)
{
    text_layer->text = text;
    layer_mark_dirty(&text_layer->layer);
}

const char *text_layer_get_text(TextLayer *text_layer)
{
    return text_layer->text;
}

void text_layer_set_background_color(TextLayer *text_layer, GColor color)
{
    text_layer->background_color = color;
    layer_mark_dirty(&text_layer->layer);
}

void text_layer_set_text_color(TextLayer *text_layer, GColor color)
{
    text_layer->text_color = color;
    layer_mark_dirty(&text_layer->layer);
}

void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode)
{
    text_layer->overflow_mode = line_mode;
    layer_mark_dirty(&text_layer->layer);
}

void text_layer_set_font(TextLayer * text_layer, GFont font)
{   
    text_layer->font = font;
    layer_mark_dirty(&text_layer->layer);
}

void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment)
{
    text_layer->text_alignment = text_alignment;
    layer_mark_dirty(&text_layer->layer);
}

GSize text_layer_get_content_size(TextLayer *text_layer)
{
    return text_layer->layer.bounds.size;
}

void text_layer_set_size(TextLayer *text_layer, const GSize max_size)
{
    text_layer->layer.frame.size = max_size;
    layer_mark_dirty(&text_layer->layer);
}

void text_layer_draw(struct Layer *layer, GContext *context)
{
    TextLayer *tlayer = (TextLayer *)layer->container;
    
//     printf("FONT: v %d lnh %d gcnt %d cp %d has %d cpb %d fis %d f %d\n",
//            (tlayer->font)->version,
//            (tlayer->font)->line_height,
//            (tlayer->font)->glyph_amount,
//            (tlayer->font)->wildcard_codepoint,
//            (tlayer->font)->hash_table_size,
//            (tlayer->font)->codepoint_bytes,
//            (tlayer->font)->fontinfo_size,
//            (tlayer->font)->features);
    context->text_color = tlayer->text_color;
    context->fill_color = tlayer->background_color;

    GRect bounds = GRect(0, 0, layer->frame.size.w, layer->frame.size.h);
    graphics_fill_rect(context, bounds, 0, GCornerNone);

    graphics_draw_text(context, tlayer->text, tlayer->font,
                       bounds, tlayer->overflow_mode,
                       tlayer->text_alignment, &tlayer->text_attributes);
}

// TODO paging...

void text_layer_enable_screen_text_flow_and_paging(TextLayer *text_layer, uint8_t inset)
{
}

void text_layer_restore_default_text_flow_and_paging(TextLayer *text_layer)
{
}

/* XXX: what's different? */
TextLayer *text_layer_legacy2_create(GRect frame)
{
    return text_layer_create(frame);
}

void text_layer_legacy2_destroy(TextLayer *layer)
{
    text_layer_destroy(layer);
}

GSize text_layer_legacy2_get_content_size(TextLayer *text_layer)
{
    return text_layer_get_content_size(text_layer);
}

Layer *text_layer_legacy2_get_layer(TextLayer *text_layer)
{
    return text_layer_get_layer(text_layer);
}


const char *text_layer_legacy2_get_text(TextLayer *text_layer)
{
    return text_layer_get_text(text_layer);
}

void text_layer_legacy2_set_background_color_2bit(TextLayer *text_layer, int color_2bit)
{
    /* XXX: not sure about this... */
    SYS_LOG("text", APP_LOG_LEVEL_INFO, "TEXT LAYER BACKGROUND COLOR -> %d\n", color_2bit);
    text_layer_set_background_color(text_layer,
        color_2bit == 0 ? GColorClear :
        color_2bit == 1 ? GColorClear :
        color_2bit == 2 ? GColorBlack :
        color_2bit == 3 ? GColorWhite :
                          GColorClear);
}

void text_layer_legacy2_set_font(TextLayer *text_layer, GFont font)
{
    text_layer_set_font(text_layer, font);
}

void text_layer_legacy2_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode)
{
    text_layer_set_overflow_mode(text_layer, line_mode);
}

void text_layer_legacy2_set_size(TextLayer *text_layer, const GSize max_size)
{
    text_layer_set_size(text_layer, max_size);
}

void text_layer_legacy2_set_text(TextLayer *text_layer, const char *text)
{
    text_layer_set_text(text_layer, text);
}

void text_layer_legacy2_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment)
{
    text_layer_set_text_alignment(text_layer, text_alignment);
}

void text_layer_legacy2_set_text_color_2bit(TextLayer *text_layer, int color_2bit)
{
    /* XXX: not sure about this... */
    SYS_LOG("text", APP_LOG_LEVEL_INFO, "TEXT LAYER TEXT COLOR -> %d\n", color_2bit);
    text_layer_set_text_color(text_layer,
        color_2bit == 0 ? GColorClear :
        color_2bit == 1 ? GColorClear :
        color_2bit == 2 ? GColorBlack :
        color_2bit == 3 ? GColorWhite :
                          GColorClear);
}

