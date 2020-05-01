/* dialog_choice_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "pin_window.h"
#include "test_defs.h"

PinWindow *pinhead;

static void pin_complete_callback(PIN pin, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Pin was %d %d %d", pin.digits[0], pin.digits[1], pin.digits[2]);
  pin_window_pop((PinWindow*)context, true);
}

bool pin_window_test_init(Window *window)
{
    return true;
}

bool pin_window_test_exec(void)
{
    pinhead = pin_window_create((PinWindowCallbacks) {
        .pin_complete = pin_complete_callback
    });    

    pin_window_push(pinhead, false);

    return true;
}

bool pin_window_test_deinit(void)
{
    return true;
}
