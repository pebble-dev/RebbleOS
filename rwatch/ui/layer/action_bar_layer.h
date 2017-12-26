#pragma once
/* layer.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "point.h"
#include "rect.h"
#include "size.h"
#include "click_config.h"
#include "bitmap_layer.h"

#define ACTION_BAR_WIDTH 30
#define NUM_ACTION_BAR_ITEMS 3
#define _ACTION_BAR_WIDTH 30

typedef struct ActionBarLayer
{
    Layer *layer;
    GBitmap *icons[NUM_ACTION_BAR_ITEMS + 1];
    GColor background_color;
    void *context;
    ClickConfigProvider *click_config_provider;
} ActionBarLayer;

ActionBarLayer *action_bar_layer_create();
void action_bar_layer_destroy(ActionBarLayer *action_bar);
Layer *action_bar_layer_get_layer(ActionBarLayer *action_bar);
void action_bar_layer_set_context(ActionBarLayer *action_bar, void *context);
void action_bar_layer_set_click_config_provider(ActionBarLayer *action_bar, ClickConfigProvider click_config_provider);

void action_bar_layer_set_icon(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon);
void action_bar_layer_set_icon_animated(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon, bool animated);
void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id);

void action_bar_layer_add_to_window(ActionBarLayer *action_bar, struct Window *window);
void action_bar_layer_remove_from_window(ActionBarLayer *action_bar);

void action_bar_layer_set_background_color(ActionBarLayer *action_bar, GColor background_color);

typedef enum {
    ActionBarLayerIconPressAnimationNone = 0,
    ActionBarLayerIconPressAnimationMoveLeft = 1,
    ActionBarLayerIconPressAnimationMoveUp = 2,
    ActionBarLayerIconPressAnimationMoveRight = 3,
    ActionBarLayerIconPressAnimationMoveDown = 4
} ActionBarLayerIconPressAnimation;

void action_bar_layer_set_icon_press_animation(ActionBarLayer *action_bar, ButtonId button_id, ActionBarLayerIconPressAnimation animation);

static void draw(Layer *layer, GContext *context);
