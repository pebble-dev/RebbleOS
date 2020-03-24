#pragma once
/* radio_button_window.h
 *
 * Radio button menu component.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"

#define RADIO_BUTTON_WINDOW_CELL_HEIGHT  44
#define RADIO_BUTTON_WINDOW_RADIO_RADIUS 6

void radio_button_window_push();

void add_radio_selection(char *selection_label);
void set_radio_selection_colors(GColor background, GColor foreground);