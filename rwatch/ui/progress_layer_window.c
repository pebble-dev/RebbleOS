#include "rebbleos.h"
#include "progress_layer_window.h"

static ProgressLayer *s_progress_layer;

static AppTimer *s_timer;
static int s_progress;

struct ProgressLayerWindow
{
  Window window;
  ProgressLayer *progress_layer;
};

static void progress_callback(void *context);

static void next_timer() {
  s_timer = app_timer_register(PROGRESS_LAYER_WINDOW_DELTA, progress_callback, NULL);
}

static void progress_callback(void *context) {

  ProgressLayerWindow *prog = (ProgressLayerWindow *) context;

  s_progress += (s_progress < 100) ? 1 : -100;
  progress_layer_set_progress(prog->progress_layer, s_progress);
  next_timer();
}

static void progresslayer_window_load(Window *window) {

  ProgressLayerWindow *prog = (ProgressLayerWindow *)window;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  prog->progress_layer = progress_layer_create(GRect((bounds.size.w - PROGRESS_LAYER_WINDOW_WIDTH) / 4, 40, PROGRESS_LAYER_WINDOW_WIDTH, 6));
  progress_layer_set_progress(prog->progress_layer, 0);
  progress_layer_set_corner_radius(prog->progress_layer, 2);
  progress_layer_set_foreground_color(prog->progress_layer, GColorWhite);
  progress_layer_set_background_color(prog->progress_layer, GColorBlack);
  layer_add_child(window_layer, prog->progress_layer);
}

static void progresslayer_window_unload(Window *window) {
  /*progress_layer_destroy(s_progress_layer);

  window_destroy(window);
  s_window = NULL;*/
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

void progresslayer_window_push(ProgressLayerWindow *prog) {
  window_stack_push(&prog->window, true);
}

ProgressLayerWindow *progresslayer_window_create()
{
  ProgressLayerWindow *prog = (ProgressLayerWindow *)app_calloc(1, sizeof(ProgressLayerWindow));

  window_ctor(&prog->window);
  prog->window.user_data = prog;
    window_set_window_handlers(&prog->window, (WindowHandlers) {
        .load = progresslayer_window_load,
        .unload = progresslayer_window_unload,
    });

  /*prog->checkbox_foreground_color = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack);
  prog->checkbox_background_color = GColorWhite;
  prog->selection_count = 0;*/

  return prog;
}