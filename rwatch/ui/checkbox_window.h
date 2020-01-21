#pragma once
/* checkbox_window.h
 *
 * Checkbox menu component.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"

#define CHECKBOX_WINDOW_NUM_ROWS    4
#define CHECKBOX_WINDOW_BOX_SIZE    12
#define CHECKBOX_WINDOW_CELL_HEIGHT 44

void checkbox_window_push();

void add_checkbox_selection(char *selection_label);
void set_checkbox_selection_colors(GColor background, GColor foreground);