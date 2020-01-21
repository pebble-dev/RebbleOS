/* radio_button_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "progress_layer_window.h"
#include "test_defs.h"

bool progress_layer_test_init(Window *window)
{
    return true;
}

bool progress_layer_test_exec(void)
{

    progress_layer_window_push();

    return true;
}

bool progress_layer_test_deinit(void)
{
    return true;
}
