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
    set_radio_selection_colors(PBL_IF_COLOR_ELSE(GColorRed, GColorBlack), GColorWhite);

    add_radio_selection("Well here we");
    add_radio_selection("are again");
    add_radio_selection("It's always");
    add_radio_selection("such a pleasure");
    add_radio_selection("(No it's not...)");

    radio_button_window_push();

    return true;
}

bool radio_button_test_deinit(void)
{
    return true;
}
