#pragma once

/* checkbox_window.h
 *
 * Checkbox menu component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"

#define CHECKBOX_WINDOW_NUM_ROWS    4
#define CHECKBOX_WINDOW_BOX_SIZE    12
#define CHECKBOX_WINDOW_CELL_HEIGHT 44

struct CheckboxWindow;
typedef struct CheckboxWindow CheckboxWindow;

typedef void (*CheckboxWindowComplete)(bool* s_selections, void *context);

typedef struct CheckboxWindowCallbacks {
  CheckboxWindowComplete checkbox_complete;
} CheckboxWindowCallbacks;

CheckboxWindow *checkbox_window_create(uint16_t max_items, CheckboxWindowCallbacks checkbox_window_callbacks);
void checkbox_window_push(CheckboxWindow *checkmate, bool animated);
void checkbox_window_pop(CheckboxWindow *checkmate, bool animated);

void checkbox_add_selection(CheckboxWindow *checkmate, char *selection_label);
void checkbox_set_selection_colors(CheckboxWindow *checkmate, GColor background, GColor foreground);