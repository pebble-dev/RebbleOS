#pragma once

/* progress_layer_window.h
 *
 * Progress layer window component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "progress_layer.h"

#define PROGRESS_LAYER_WINDOW_DELTA 33
#define PROGRESS_LAYER_WINDOW_WIDTH 80

struct ProgressLayerWindow;
typedef struct ProgressLayerWindow ProgressLayerWindow;

void progresslayer_window_push(ProgressLayerWindow *prog);

ProgressLayerWindow *progresslayer_window_create();