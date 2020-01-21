/* radio_button_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "radio_button_window.h"
#include "test_defs.h"

bool radio_button_test_init(Window *window)
{
    return true;
}

bool radio_button_test_exec(void)
{

    radio_button_window_push();

    return true;
}

bool radio_button_test_deinit(void)
{
    return true;
}
