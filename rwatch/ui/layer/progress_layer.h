#pragma once

/* progress_layer.h
 *
 * Progress layer component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"

typedef Layer ProgressLayer;

ProgressLayer* progress_layer_create(GRect frame);
void progress_layer_destroy(ProgressLayer* progress_layer);
void progress_layer_increment_progress(ProgressLayer* progress_layer, int16_t progress);
void progress_layer_set_progress(ProgressLayer* progress_layer, int16_t progress_percent);
void progress_layer_set_corner_radius(ProgressLayer* progress_layer, uint16_t corner_radius);
void progress_layer_set_foreground_color(ProgressLayer* progress_layer, GColor color);
void progress_layer_set_background_color(ProgressLayer* progress_layer, GColor color);