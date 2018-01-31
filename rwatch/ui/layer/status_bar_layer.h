#pragma once
/* action_bar_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 */

#include "point.h"
#include "rect.h"
#include "size.h"
#include "bitmap_layer.h"

#define STATUS_BAR_LAYER_HEIGHT (PBL_PLATFORM_SWITCH(16, 16, 24, 16, 20))

typedef enum StatusBarLayerSeparatorMode
{
    StatusBarLayerSeparatorModeNone,
    StatusBarLayerSeparatorModeDotted
} StatusBarLayerSeparatorMode;

typedef struct StatusBarLayer
{
    Layer layer;
    GColor foreground_color;
    GColor background_color;
    StatusBarLayerSeparatorMode separator_mode;
    const char *text;
    struct tm last_time;
    CoreTimer timer;
} StatusBarLayer;

StatusBarLayer* status_bar_layer_create(void);
void status_bar_layer_destroy(StatusBarLayer *status_bar_layer);

Layer *status_bar_layer_get_layer(StatusBarLayer *status_bar_layer);
GColor status_bar_layer_get_background_color(StatusBarLayer *status_bar_layer);
GColor status_bar_layer_get_foreground_color(StatusBarLayer *status_bar_layer);
void status_bar_layer_set_colors(StatusBarLayer *status_bar_layer, GColor background, GColor foreground);
void status_bar_layer_set_separator_mode(StatusBarLayer *status_bar_layer, StatusBarLayerSeparatorMode mode);
void status_bar_layer_set_text(StatusBarLayer *status_bar, const char *status_text);
