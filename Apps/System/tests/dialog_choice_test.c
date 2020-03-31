/* dialog_choice_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "dialog_choice_window.h"
#include "test_defs.h"

DialogchoiceWindow *dial;

bool dialog_choice_test_init(Window *window)
{
    return true;
}

bool dialog_choice_test_exec(void)
{

    dial = dialogchoice_window_create();

    dialogchoice_window_set_message(dial, "Set phasers to kill?");
    dialog_choice_window_push(dial);

    return true;
}

bool dialog_choice_test_deinit(void)
{
    return true;
}
