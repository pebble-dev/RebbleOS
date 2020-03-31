#pragma once

/* radio_button_window.h
 *
 * Radio button menu component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"

#define RADIO_BUTTON_WINDOW_CELL_HEIGHT  44
#define RADIO_BUTTON_WINDOW_RADIO_RADIUS 6

struct RadiobuttonWindow;
typedef struct RadiobuttonWindow RadiobuttonWindow;

void radio_button_window_push(RadiobuttonWindow *radio_star);

RadiobuttonWindow *radiobutton_window_create(uint16_t max_items);
void radiobutton_add_selection(RadiobuttonWindow *radio_star, char *selection_label);
void set_radiobutton_selection_colors(RadiobuttonWindow *radio_star, GColor background, GColor foreground);