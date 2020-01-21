/* dialog_choice_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "dialog_choice_window.h"
#include "test_defs.h"

bool dialog_choice_test_init(Window *window)
{
    return true;
}

bool dialog_choice_test_exec(void)
{

    dialog_choice_window_push();

    return true;
}

bool dialog_choice_test_deinit(void)
{
    return true;
}
