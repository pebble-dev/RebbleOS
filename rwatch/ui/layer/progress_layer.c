/* progress_layer.c
 *
 * Progress layer component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "progress_layer.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

typedef struct {
  int16_t progress_percent;
  int16_t corner_radius;
  GColor foreground_color;
  GColor background_color;
} ProgressLayerData;

static int16_t scale_progress_bar_width_px(unsigned int progress_percent, int16_t rect_width_px) {
  return ((progress_percent * (rect_width_px)) / 100);
}

static void progress_layer_update_proc(ProgressLayer* progress_layer, GContext* ctx) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  GRect bounds = layer_get_bounds(progress_layer);

  int16_t progress_bar_width_px = scale_progress_bar_width_px(data->progress_percent, bounds.size.w);
  GRect progress_bar = GRect(bounds.origin.x, bounds.origin.y, progress_bar_width_px, bounds.size.h);

  graphics_context_set_fill_color(ctx, data->background_color);
  graphics_fill_rect(ctx, bounds, data->corner_radius, 5);

  graphics_context_set_fill_color(ctx, data->foreground_color);
  graphics_fill_rect(ctx, progress_bar, data->corner_radius, 5);

#ifdef PBL_PLATFORM_APLITE
  graphics_context_set_stroke_color(ctx, data->background_color);
  graphics_draw_rect(ctx, progress_bar);
#endif
}

ProgressLayer* progress_layer_create(GRect frame) {
  ProgressLayer *progress_layer = layer_create_with_data(frame, sizeof(ProgressLayerData));
  layer_set_update_proc(progress_layer, progress_layer_update_proc);
  layer_mark_dirty(progress_layer);

  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = 0;
  data->corner_radius = 1;
  data->foreground_color = GColorBlack;
  data->background_color = GColorWhite;

  return progress_layer;
}

void progress_layer_destroy(ProgressLayer* progress_layer) {
  if (progress_layer) {
    layer_destroy(progress_layer);
  }
}

void progress_layer_increment_progress(ProgressLayer* progress_layer, int16_t progress) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = MIN(100, data->progress_percent + progress);
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_progress(ProgressLayer* progress_layer, int16_t progress_percent) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = MIN(100, progress_percent);
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_corner_radius(ProgressLayer* progress_layer, uint16_t corner_radius) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->corner_radius = corner_radius;
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_foreground_color(ProgressLayer* progress_layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->foreground_color = color;
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_background_color(ProgressLayer* progress_layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->background_color = color;
  layer_mark_dirty(progress_layer);
}