#pragma once

#include "librebble.h"

#define MAX_SELECTION_LAYER_CELLS 3

typedef char* (*SelectionLayerGetCellText)(int index, void *context);

typedef void (*SelectionLayerCompleteCallback)(void *context);

typedef void (*SelectionLayerIncrementCallback)(int selected_cell_idx, uint8_t reapeating_count, void *context);

typedef void (*SelectionLayerDecrementCallback)(int selected_cell_idx, uint8_t reapeating_count, void *context);

typedef struct SelectionLayerCallbacks {
  SelectionLayerGetCellText get_cell_text;
  SelectionLayerCompleteCallback complete;
  SelectionLayerIncrementCallback increment;
  SelectionLayerDecrementCallback decrement;
} SelectionLayerCallbacks;

typedef struct SelectionLayerData {
  int num_cells;
  int cell_widths[MAX_SELECTION_LAYER_CELLS];
  int cell_padding;
  int selected_cell_idx;

  // If is_active = false the the selected cell will become invalid, and any clicks will be ignored
  bool is_active;

  GFont font;
  GColor inactive_background_color;
  GColor active_background_color;

  SelectionLayerCallbacks callbacks;
  void *context;

  // Animation stuff
  Animation *value_change_animation;
  bool bump_is_upwards;
  int bump_text_anim_progress;
  AnimationImplementation bump_text_impl;
  int bump_settle_anim_progress;
  AnimationImplementation bump_settle_anim_impl;

  Animation *next_cell_animation;
  bool slide_is_forward;
  int slide_amin_progress;
  AnimationImplementation slide_amin_impl;
  int slide_settle_anim_progress;
  AnimationImplementation slide_settle_anim_impl;
} SelectionLayerData;

Layer* selection_layer_create(GRect frame, int num_cells);

void selection_layer_destroy(Layer* layer);

void selection_layer_set_cell_width(Layer *layer, int cell_idx, int width);

void selection_layer_set_font(Layer *layer, GFont font);

void selection_layer_set_inactive_bg_color(Layer *layer, GColor color);

void selection_layer_set_active_bg_color(Layer *layer, GColor color);

void selection_layer_set_cell_padding(Layer *layer, int padding);

// When transitioning from inactive -> active, the selected cell will be index 0
void selection_layer_set_active(Layer *layer, bool is_active);

void selection_layer_set_click_config_onto_window(Layer *layer, struct Window *window);

void selection_layer_set_callbacks(Layer *layer, void *context, SelectionLayerCallbacks callbacks);