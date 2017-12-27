#pragma once
/* action_bar_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "point.h"
#include "rect.h"
#include "size.h"
#include "bitmap_layer.h"

#define STATUS_BAR_LAYER_HEIGHT 16
#define _STATUS_BAR_LAYER_HEIGHT 16

typedef enum StatusBarLayerSeparatorMode
{
    StatusBarLayerSeparatorModeNone,
    StatusBarLayerSeparatorModeDotted
} StatusBarLayerSeparatorMode;

typedef struct StatusBarLayer
{
    Layer *layer;
    GColor text_color;
    GColor background_color;
    StatusBarLayerSeparatorMode *separator_mode;
} StatusBarLayer;

StatusBarLayer* status_bar_layer_create(void);
void status_bar_layer_destroy(StatusBarLayer * status_bar_layer);

Layer *status_bar_layer_get_layer(StatusBarLayer * status_bar_layer);
GColor status_bar_layer_get_background_color(StatusBarLayer * status_bar_layer);
GColor status_bar_layer_get_foreground_color(StatusBarLayer * status_bar_layer);
void status_bar_layer_set_colors(StatusBarLayer * status_bar_layer, GColor background, GColor foreground);

void status_bar_layer_set_separator_mode(StatusBarLayer * status_bar_layer, StatusBarLayerSeparatorMode mode);


static void draw(Layer *layer, GContext *context);
