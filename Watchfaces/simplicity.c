/* From: https://github.com/pebble-examples/simplicity
 * 
 * Copyright (c) 2016 Pebble Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "librebble.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

static Window *s_main_window;
static TextLayer *s_date_layer, *s_time_layer;
static Layer *s_window_layer, *s_shifting_layer, *s_line_layer;
static int16_t s_obstruction = 0;

// Move the shifting layer based on the obstruction size
static void prv_resposition_layers() {
  GRect full_bounds = layer_get_bounds(s_window_layer);
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);

  // How big is the obstruction?
  s_obstruction = full_bounds.size.h - bounds.size.h;

  // Move the shifting layer
  GRect frame = layer_get_frame(s_shifting_layer);
  frame.origin.y = bounds.origin.y - s_obstruction;
  layer_set_frame(s_shifting_layer, frame);
}

// Event fires frequently, while obstruction is appearing or disappearing
static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  prv_resposition_layers();
}

// Event fires once, after obstruction appears or disappears
static void prv_unobstructed_did_change(void *context) {
  prv_resposition_layers();
}

// Clock tick event
static void prv_handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  static char s_time_text[] = "00:00";
  static char s_date_text[] = "Xxxxxxxxx 00";

  char *time_format = pbl_clock_is_24h_style() ? "%R" : "%I:%M";
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!pbl_clock_is_24h_style() && (s_time_text[0] == '0')) {
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
  text_layer_set_text(s_time_layer, s_time_text);

  // Update day, only if it's changed
  if (units_changed & DAY_UNIT) {
    strftime(s_date_text, sizeof(s_date_text), "%B %e", tick_time);
    text_layer_set_text(s_date_layer, s_date_text);
  }
}

// Draw the horizontal line
static void prv_line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

// Window Load
static void prv_main_window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  GRect full_bounds = layer_get_bounds(s_window_layer);
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);

  // This whole layer will shift when the unobstructed size changes
  s_shifting_layer = layer_create(full_bounds);
  layer_add_child(s_window_layer, s_shifting_layer);

  // The Date
  s_date_layer = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(bounds.origin.x, 68, bounds.size.w, 100),
    GRect(8, 68, 136, 100)));
  text_layer_set_text_alignment(s_date_layer, PBL_IF_ROUND_ELSE(
    GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(s_shifting_layer, text_layer_get_layer(s_date_layer));

  // The Time
  s_time_layer = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(bounds.origin.x, 92, bounds.size.w, 76),
    GRect(7, 92, 137, 76)));
  text_layer_set_text_alignment(s_time_layer, PBL_IF_ROUND_ELSE(
    GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(s_shifting_layer, text_layer_get_layer(s_time_layer));

  // The horizontal line
  GRect line_frame = GRect(8, 97, bounds.size.w - 16, 2);
  s_line_layer = layer_create(line_frame);
  layer_set_update_proc(s_line_layer, prv_line_layer_update_callback);
  layer_add_child(s_shifting_layer, s_line_layer);

#if 0
  // Subscribe to the unobstructed area events
  UnobstructedAreaHandlers handlers = {
    .change = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);
#endif

  // Force position update
  prv_resposition_layers();
}

// Window Unload
static void prv_main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);

  layer_destroy(s_line_layer);
  layer_destroy(s_shifting_layer);
}

static void prv_init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = prv_main_window_load,
    .unload = prv_main_window_unload,
  });
  window_stack_push(s_main_window, true);

#if 0
  setlocale(LC_ALL, "");
#endif

  tick_timer_service_subscribe(MINUTE_UNIT, prv_handle_minute_tick);

  // Prevent starting blank
  time_t now;
  rcore_time_ms(&now, NULL);
  struct tm *t = localtime(&now);
  prv_handle_minute_tick(t, DAY_UNIT);
}

static void prv_deinit() {
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
}

void simplicity_main() {
  prv_init();
  app_event_loop();
  prv_deinit();
}
