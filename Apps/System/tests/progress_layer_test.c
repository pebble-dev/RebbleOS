/* radio_button_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "progress_layer_window.h"
#include "test_defs.h"

ProgressLayerWindow *prog;

bool progress_layer_test_init(Window *window)
{
    return true;
}

bool progress_layer_test_exec(void)
{

    prog = progresslayer_window_create();

    progresslayer_window_push(prog);

    return true;
}

bool progress_layer_test_deinit(void)
{
    return true;
}
