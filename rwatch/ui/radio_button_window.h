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

typedef void (*RadiobuttonWindowComplete)(bool* s_selections, void *context);

typedef struct RadiobuttonWindowCallbacks {
  RadiobuttonWindowComplete radiobutton_complete;
} RadiobuttonWindowCallbacks;

RadiobuttonWindow *radiobutton_window_create(uint16_t max_items, RadiobuttonWindowCallbacks radiobutton_window_callbacks);
void radio_button_window_push(RadiobuttonWindow *radio_star, bool animated);
void radio_button_window_pop(RadiobuttonWindow *radio_star, bool animated);

void radiobutton_add_selection(RadiobuttonWindow *radio_star, char *selection_label);
void radiobutton_set_selection_colors(RadiobuttonWindow *radio_star, GColor background, GColor foreground);