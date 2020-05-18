// Pebble UI component adapted for modular use by Eric Phillips

#include "selection_layer.h"

// Look and feel
#define DEFAULT_CELL_PADDING 10
#define DEFAULT_SELECTED_INDEX 0
#define DEFAULT_FONT FONT_KEY_GOTHIC_28_BOLD
#define DEFAULT_ACTIVE_COLOR GColorWhite
#define DEFAULT_INACTIVE_COLOR PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)

#define BUTTON_HOLD_REPEAT_MS 100
#define SETTLE_HEIGHT_DIFF 6

// Animation
#define BUMP_TEXT_DURATION_MS 107
#define BUMP_SETTLE_DURATION_MS 214
#define SLIDE_DURATION_MS 107
#define SLIDE_SETTLE_DURATION_MS 179

// Function prototypes
static Animation* prv_create_bump_settle_animation(Layer *layer);
static Animation* prv_create_slide_settle_animation(Layer *layer);

static int prv_get_pixels_for_bump_settle(int anim_percent_complete) {
  if (anim_percent_complete) {
    return SETTLE_HEIGHT_DIFF - ((SETTLE_HEIGHT_DIFF * anim_percent_complete) / 100);
  } else {
    return 0;
  }
}

static int prv_get_font_top_padding(GFont font) {
  if (font == fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) {
    return 10;
  } else if (font == fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) {
    return 10;
  } else {
    return 0;
  }
}

static int prv_get_y_offset_which_vertically_centers_font(GFont font, int height) {
  int font_height = 0;
  int font_top_padding = prv_get_font_top_padding(font);
  if (font == fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) {
    font_height = 18;
  } else if (font == fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) {
    font_height = 14;
  }

  return (height / 2) - (font_height / 2) - font_top_padding;
}

static void prv_draw_cell_backgrounds(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  // Loop over each cell and draw the background rectangles
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->cell_widths[i] == 0) {
      continue;
    }

    int y_offset = 0;
    if (data->selected_cell_idx == i && data->bump_is_upwards) {
      y_offset = -prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
    }

    int height = layer_get_bounds(layer).size.h;
    if (data->selected_cell_idx == i) {
      height += prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
    }

    const GRect rect = GRect(current_x_offset, y_offset, data->cell_widths[i], height);

    GColor bg_color = data->inactive_background_color;

    if (data->selected_cell_idx == i && !data->slide_amin_progress) {
      bg_color = data->active_background_color;
    }
    graphics_context_set_fill_color(ctx, bg_color);
    graphics_fill_rect(ctx, rect, 1, GCornerNone);

    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_slider_slide(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);

  int starting_x_offset = 0;
  for (int i = 0; i < data->num_cells; i++) {
    if (data->selected_cell_idx == i) {
      break;
    }
    starting_x_offset += data->cell_widths[i] + data->cell_padding;
  }

  int next_cell_width = data->cell_widths[data->selected_cell_idx + 1];
  if (!data->slide_is_forward) {
    next_cell_width = data->cell_widths[data->selected_cell_idx - 1];
  }

  int slide_distance = next_cell_width + data->cell_padding;
  int current_slide_distance = (slide_distance * data->slide_amin_progress) / 100;
  if (!data->slide_is_forward) {
    current_slide_distance = -current_slide_distance;
  }

  int current_x_offset = starting_x_offset + current_slide_distance;
  int cur_cell_width = data->cell_widths[data->selected_cell_idx];
  int total_cell_width_change = next_cell_width - cur_cell_width + data->cell_padding;
  int current_cell_width_change = (total_cell_width_change * (int) data->slide_amin_progress) / 100;
  int current_cell_width = cur_cell_width + current_cell_width_change;
  if (!data->slide_is_forward) {
    current_x_offset -= current_cell_width_change;
  }

  GRect rect = GRect(current_x_offset, 0, current_cell_width, layer_get_bounds(layer).size.h);

  graphics_context_set_fill_color(ctx, data->active_background_color);
  graphics_fill_rect(ctx, rect, 1, GCornerNone);
}

static void prv_draw_slider_settle(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  int starting_x_offset = 0;
  for (int i = 0; i < data->num_cells; i++) {
    if (data->selected_cell_idx == i) {
      break;
    }
    starting_x_offset += data->cell_widths[i] + data->cell_padding;
  }

  int x_offset = starting_x_offset;
  if (data->slide_is_forward) {
    x_offset += data->cell_widths[data->selected_cell_idx];
  }

  int current_width = (data->cell_padding * data->slide_settle_anim_progress) / 100;
  if (!data->slide_is_forward) {
    x_offset -= current_width;
  }

  GRect rect = GRect(x_offset, 0, current_width, layer_get_bounds(layer).size.h);

  graphics_context_set_fill_color(ctx, data->active_background_color);
  graphics_fill_rect(ctx, rect, 1, GCornerNone);
}

static void prv_draw_text(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  for (int i = 0, current_x_offset = 0; i < data->num_cells; i++) {
    if (data->callbacks.get_cell_text) {
      char *text = data->callbacks.get_cell_text(i, data->context);
      if (text) {
        int height = layer_get_bounds(layer).size.h;
        if (data->selected_cell_idx == i) {
          height += prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
        }
        int y_offset = prv_get_y_offset_which_vertically_centers_font(data->font, height);

        if (data->selected_cell_idx == i && data->bump_is_upwards) {
          y_offset -= prv_get_pixels_for_bump_settle(data->bump_settle_anim_progress);
        }

        if (data->selected_cell_idx == i) {
          int delta = (data->bump_text_anim_progress * prv_get_font_top_padding(data->font)) / 100;
          if (data->bump_is_upwards) {
            delta *= -1;
          }
          y_offset += delta;
        }

        GRect rect = GRect(current_x_offset, y_offset, data->cell_widths[i], height);
        graphics_draw_text(ctx, text, data->font, rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
      }
    }

    current_x_offset += data->cell_widths[i] + data->cell_padding;
  }
}

static void prv_draw_selection_layer(Layer *layer, GContext *ctx) {
  SelectionLayerData *data = layer_get_data(layer);
  prv_draw_cell_backgrounds(layer, ctx);

  if (data->slide_amin_progress) {
    prv_draw_slider_slide(layer, ctx);
  }
  if (data->slide_settle_anim_progress) {
    prv_draw_slider_settle(layer, ctx);
  }

  prv_draw_text(layer, ctx);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Increment / Decrement Animation

//! This animation causes a the active cell to "bump" when the user presses the up button.
//! This animation has two parts:
//! 1) The "text to cell edge"
//! 2) The "background settle"

//! The "text to cell edge" (bump_text) moves the text until it hits the top / bottom of the cell.

//! The "background settle" (bump_settle) is a reaction to the "text to cell edge" animation.
//! The top of the cell immediately expands down giving the impression that the text "pushed" the
//! cell making it bigger. The cell then shrinks / settles back to its original height
//! with the text vertically centered

static void prv_bump_text_impl(struct Animation *animation, const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->bump_text_anim_progress = (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_bump_text_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->bump_text_anim_progress = 0;

  if (data->bump_is_upwards == true) {
    data->callbacks.increment(data->selected_cell_idx, 1, data->context);
  } else {
    data->callbacks.decrement(data->selected_cell_idx, 1, data->context);
  }

  animation_destroy(animation);

  Animation *bump_settle = prv_create_bump_settle_animation(layer);
  animation_schedule(bump_settle);
}

static void prv_bump_settle_impl(struct Animation *animation, const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->bump_settle_anim_progress = (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_bump_settle_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->bump_settle_anim_progress = 0;
  animation_destroy(animation);
}

static Animation* prv_create_bump_text_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);

  PropertyAnimation *bump_text_anim = property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(bump_text_anim);
  animation_set_curve(animation, AnimationCurveEaseIn);
  animation_set_duration(animation, BUMP_TEXT_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_bump_text_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->bump_text_impl = (AnimationImplementation) {
    .update = prv_bump_text_impl,
  };
  animation_set_implementation(animation, &data->bump_text_impl);

  return animation;
}

static Animation* prv_create_bump_settle_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);

  PropertyAnimation *bump_settle_anim = property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(bump_settle_anim);
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_duration(animation, BUMP_SETTLE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_bump_settle_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->bump_settle_anim_impl = (AnimationImplementation) {
    .update = prv_bump_settle_impl,
  };
  animation_set_implementation(animation, &data->bump_settle_anim_impl);

  return animation;
}

static void prv_run_value_change_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);

  Animation *bump_text = prv_create_bump_text_animation(layer);
  Animation *bump_settle = prv_create_bump_settle_animation(layer);
  data->value_change_animation = animation_sequence_create(bump_text, bump_settle, NULL);
  animation_schedule(data->value_change_animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Slide Animation

//! This animation moves the "selection box" (active color) to the next cell to the right.
//! This animation has two parts:
//! 1) The "move and expand"
//! 2) The "settle"

//! The "move and expand" (slide) moves the selection box from the currently active cell to
//! the next cell to the right. At the same time the width is changed to be the size of the
//! next cell plus the size of the padding. This creates an overshoot effect.

//! The "settle" (slide_settle) removes the extra width that was added in the "move and expand"
//! step.

static void prv_slide_impl(struct Animation *animation, const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->slide_amin_progress = (100 * distance_normalized) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(layer);
}

static void prv_slide_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->slide_amin_progress = 0;

  if (data->slide_is_forward) {
    data->selected_cell_idx++;
  } else {
    data->selected_cell_idx--;
  }

  animation_destroy(animation);

  Animation *settle_animation = prv_create_slide_settle_animation(layer);
  animation_schedule(settle_animation);
}

static void prv_slide_settle_impl(struct Animation *animation, const AnimationProgress distance_normalized) {
  Layer *layer = (Layer*)animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->slide_settle_anim_progress = 100 - ((100 * distance_normalized) / ANIMATION_NORMALIZED_MAX);
  layer_mark_dirty(layer);
}

static void prv_slide_settle_stopped(Animation *animation, bool finished, void *context) {
  Layer *layer = (Layer*) animation_get_context(animation);
  SelectionLayerData *data = layer_get_data(layer);

  data->slide_settle_anim_progress = 0;
  animation_destroy(animation);
}

static Animation* prv_create_slide_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);

  PropertyAnimation *slide_amin = property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(slide_amin);
  animation_set_curve(animation, AnimationCurveEaseIn);
  animation_set_duration(animation, SLIDE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_slide_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->slide_amin_impl = (AnimationImplementation) {
    .update = prv_slide_impl,
  };
  animation_set_implementation(animation, &data->slide_amin_impl);

  return animation;
}

static Animation* prv_create_slide_settle_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);
  PropertyAnimation *slide_settle_anim = property_animation_create_layer_frame(layer, NULL, NULL);
  Animation *animation = property_animation_get_animation(slide_settle_anim);
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_duration(animation, SLIDE_SETTLE_DURATION_MS);
  AnimationHandlers anim_handler = {
      .stopped = prv_slide_settle_stopped,
  };
  animation_set_handlers(animation, anim_handler, layer);

  data->slide_settle_anim_impl = (AnimationImplementation) {
    .update = prv_slide_settle_impl,
  };
  animation_set_implementation(animation, &data->slide_settle_anim_impl);

  return animation;
}

static void prv_run_slide_animation(Layer *layer) {
  SelectionLayerData *data = layer_get_data(layer);

  Animation *over_animation = prv_create_slide_animation(layer);
  Animation *settle_animation = prv_create_slide_settle_animation(layer);
  data->next_cell_animation = animation_sequence_create(over_animation, settle_animation, NULL);
  animation_schedule(data->next_cell_animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Click handlers

void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
/*
  if (data->is_active) {
    if (click_recognizer_is_repeating(recognizer)) {
      // Don't animate if the button is being held down. Just update the text
      data->callbacks.increment(data->selected_cell_idx, click_number_of_clicks_counted(recognizer), data->context);
      layer_mark_dirty(layer);
    } else {
      data->bump_is_upwards = true;
      prv_run_value_change_animation(layer);
    }
  }
  */
}

void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);
/*
  if (data->is_active) {
    if (click_recognizer_is_repeating(recognizer)) {
      // Don't animate if the button is being held down. Just update the text
      data->callbacks.decrement(data->selected_cell_idx, click_number_of_clicks_counted(recognizer), data->context);
      layer_mark_dirty(layer);
    } else {
      data->bump_is_upwards = false;
      prv_run_value_change_animation(layer);
    }
  }
  */
}

void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
    animation_unschedule(data->next_cell_animation);
    if (data->selected_cell_idx >= data->num_cells - 1) {
      data->selected_cell_idx = 0;
      data->callbacks.complete(data->context);
    } else {
      data->slide_is_forward = true;
      prv_run_slide_animation(layer);
    }
  }
}

void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  Layer *layer = (Layer*)context;
  SelectionLayerData *data = layer_get_data(layer);

  if (data->is_active) {
    animation_unschedule(data->next_cell_animation);
    if (data->selected_cell_idx == 0) {
      data->selected_cell_idx = 0;
      window_stack_pop(true);
    } else {
      data->slide_is_forward = false;
      prv_run_slide_animation(layer);
    }
  }
}

static void prv_click_config_provider(Layer *layer) {
  window_set_click_context(BUTTON_ID_UP, layer);
  window_set_click_context(BUTTON_ID_DOWN, layer);
  window_set_click_context(BUTTON_ID_SELECT, layer);
  window_set_click_context(BUTTON_ID_BACK, layer);

  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS, prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! API

static Layer* selection_layer_init(SelectionLayerData *selection_layer_, GRect frame, int num_cells) {
  Layer *layer = layer_create_with_data(frame, sizeof(SelectionLayerData));
  SelectionLayerData *selection_layer_data = layer_get_data(layer);

  if (num_cells > MAX_SELECTION_LAYER_CELLS) {
    num_cells = MAX_SELECTION_LAYER_CELLS;
  }

  // Set layer defaults
  *selection_layer_data = (SelectionLayerData) {
    .active_background_color = DEFAULT_ACTIVE_COLOR,
    .inactive_background_color = DEFAULT_INACTIVE_COLOR,
    .num_cells = num_cells,
    .cell_padding = DEFAULT_CELL_PADDING,
    .selected_cell_idx = DEFAULT_SELECTED_INDEX,
    .font = fonts_get_system_font(DEFAULT_FONT),
    .is_active = true,
  };
  for (int i = 0; i < num_cells; i++) {
    selection_layer_data->cell_widths[i] = 0;
  }
  layer_set_frame(layer, frame);
  layer_set_clips(layer, false);
  layer_set_update_proc(layer, (LayerUpdateProc)prv_draw_selection_layer);

  return layer;
}

Layer* selection_layer_create(GRect frame, int num_cells) {
  SelectionLayerData *selection_layer_data = NULL;
  return selection_layer_init(selection_layer_data, frame, num_cells);
}

static void selection_layer_deinit(Layer* layer) {
  layer_destroy(layer);
}

void selection_layer_destroy(Layer* layer) {
  SelectionLayerData *data = layer_get_data(layer);

  animation_unschedule_all();
  if (data) {
    selection_layer_deinit(layer);
  }
}

void selection_layer_set_cell_width(Layer *layer, int idx, int width) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data && idx < data->num_cells) {
    data->cell_widths[idx] = width;
  }
}

void selection_layer_set_font(Layer *layer, GFont font) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->font = font;
  }
}

void selection_layer_set_inactive_bg_color(Layer *layer, GColor color) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->inactive_background_color = color;
  }
}

void selection_layer_set_active_bg_color(Layer *layer, GColor color) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->active_background_color = color;
  }
}

void selection_layer_set_cell_padding(Layer *layer, int padding) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    data->cell_padding = padding;
  }
}

void selection_layer_set_active(Layer *layer, bool is_active) {
  SelectionLayerData *data = layer_get_data(layer);

  if (data) {
    if (is_active && !data->is_active) {
      data->selected_cell_idx = 0;
    } if (!is_active && data->is_active) {
      data->selected_cell_idx = MAX_SELECTION_LAYER_CELLS + 1;
    }

    data->is_active = is_active;
    layer_mark_dirty(layer);
  }
}

void selection_layer_set_click_config_onto_window(Layer *layer, struct Window *window) {
  if (layer && window) {
    window_set_click_config_provider_with_context(window, (ClickConfigProvider)prv_click_config_provider, layer);
  }
}

void selection_layer_set_callbacks(Layer *layer, void *context, SelectionLayerCallbacks callbacks) {
  SelectionLayerData *data = layer_get_data(layer);
  data->callbacks = callbacks;
  data->context = context;
}