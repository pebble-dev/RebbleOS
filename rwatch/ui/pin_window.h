#pragma once

#include "librebble.h"

#define PIN_WINDOW_NUM_CELLS 3
#define PIN_WINDOW_MAX_VALUE 9
#define PIN_WINDOW_SIZE GSize(128, 34)

typedef struct {
  int digits[PIN_WINDOW_NUM_CELLS];
} PIN;

typedef void (*PinWindowComplete)(PIN pin, void *context);

typedef struct PinWindowCallbacks {
    PinWindowComplete pin_complete;
} PinWindowCallbacks;

typedef struct {
  Window *window;
  TextLayer *main_text, *sub_text;
  Layer *selection;
  GColor highlight_color;
  StatusBarLayer *status;
  PinWindowCallbacks callbacks;

  PIN pin;
  char field_buffs[PIN_WINDOW_NUM_CELLS][2];
  int8_t field_selection;
} PinWindow;

/*
 * Creates a new PinWindow in memory but does not push it into view
 *  pin_window_callbacks: callbacks for communication
 *  returns: a pointer to a new PinWindow structure
 */
PinWindow* pin_window_create(PinWindowCallbacks pin_window_callbacks);

/*
 * Destroys an existing PinWindow
 *  pin_window: a pointer to the PinWindow being destroyed
 */
void pin_window_destroy(PinWindow *pin_window);

/*
 * Push the window onto the stack
 *  pin_window: a pointer to the PinWindow being pushed
 *  animated: whether to animate the push or not
 */
void pin_window_push(PinWindow *pin_window, bool animated);

/*
 * Pop the window off the stack
 *  pin_window: a pointer to the PinWindow to pop
 *  animated: whether to animate the pop or not
 */
void pin_window_pop(PinWindow *pin_window, bool animated);

/*
 * Gets whether it is the topmost window or not
 *  pin_window: a pointer to the PinWindow being checked
 *  returns: a boolean indicating if it is the topmost window
 */
bool pin_window_get_topmost_window(PinWindow *pin_window);

/*
 * Sets the over-all color scheme of the window
 *  color: the GColor to set the highlight to
 */
void pin_window_set_highlight_color(PinWindow *pin_window, GColor color);