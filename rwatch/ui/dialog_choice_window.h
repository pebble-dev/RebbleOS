#pragma once
/* dialog_choice_window.h
 *
 * Dialog choice component. Adapted from Pebble UI Examples.
 *
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

 #include "librebble.h"

struct DialogChoiceWindow;
typedef struct DialogChoiceWindow DialogChoiceWindow;

typedef void (*DialogChoiceWindowComplete)(bool* s_selections, void *context);

typedef struct DialogChoiceWindowCallbacks {
  DialogChoiceWindowComplete dialog_choice_complete;
} DialogChoiceWindowCallbacks;

void dialog_choice_window_push(DialogChoiceWindow *dial, bool animated);
void dialogchoice_window_set_message(DialogChoiceWindow *dial, char dialogchoice_window_message);
DialogChoiceWindow *dialogchoice_window_create(DialogChoiceWindowCallbacks dialog_choice_window_callbacks);
