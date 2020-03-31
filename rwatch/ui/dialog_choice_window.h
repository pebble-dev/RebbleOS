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

struct DialogchoiceWindow;
typedef struct DialogchoiceWindow DialogchoiceWindow;

void dialog_choice_window_push(DialogchoiceWindow *dial);
void dialogchoice_window_set_message(DialogchoiceWindow *dial, char dialogchoice_window_message);
DialogchoiceWindow *dialogchoice_window_create();
