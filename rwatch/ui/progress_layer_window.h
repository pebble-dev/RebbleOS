#pragma once

#include "librebble.h"
#include "progress_layer.h"

#define PROGRESS_LAYER_WINDOW_DELTA 33
#define PROGRESS_LAYER_WINDOW_WIDTH 80

struct ProgressLayerWindow;
typedef struct ProgressLayerWindow ProgressLayerWindow;

void progresslayer_window_push(ProgressLayerWindow *prog);

ProgressLayerWindow *progresslayer_window_create();