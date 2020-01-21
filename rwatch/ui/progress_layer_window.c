#include "rebbleos.h"
#include "progress_layer_window.h"

static Window *s_window;
static ProgressLayer *s_progress_layer;

static AppTimer *s_timer;
static int s_progress;

static void progress_callback(void *context);

static void next_timer() {
  s_timer = app_timer_register(PROGRESS_LAYER_WINDOW_DELTA, progress_callback, NULL);
}

static void progress_callback(void *context) {
  s_progress += (s_progress < 100) ? 1 : -100;
  progress_layer_set_progress(s_progress_layer, s_progress);
  next_timer();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_progress_layer = progress_layer_create(GRect((bounds.size.w - PROGRESS_LAYER_WINDOW_WIDTH) / 4, 40, PROGRESS_LAYER_WINDOW_WIDTH, 6));
  progress_layer_set_progress(s_progress_layer, 0);
  progress_layer_set_corner_radius(s_progress_layer, 2);
  progress_layer_set_foreground_color(s_progress_layer, GColorWhite);
  progress_layer_set_background_color(s_progress_layer, GColorBlack);
  layer_add_child(window_layer, s_progress_layer);
}

static void window_unload(Window *window) {
  progress_layer_destroy(s_progress_layer);

  window_destroy(window);
  s_window = NULL;
}

static void window_appear(Window *window) {
  s_progress = 0;
  next_timer();
}

static void window_disappear(Window *window) {
  if(s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

void progress_layer_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .appear = window_appear,
      .disappear = window_disappear,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}