/* radio_button_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "radio_button_window.h"
#include "test_defs.h"

RadiobuttonWindow *radio_star;

bool radio_button_test_init(Window *window)
{
    return true;
}

bool radio_button_test_exec(void)
{

    radio_star = radiobutton_window_create(4);

    radiobutton_add_selection(radio_star, "Well here we");
    radiobutton_add_selection(radio_star, "are again");
    radiobutton_add_selection(radio_star, "It's always");
    radiobutton_add_selection(radio_star, "such a pleasure");
    radiobutton_add_selection(radio_star, "(no it's not)");

    set_radiobutton_selection_colors(radio_star, PBL_IF_COLOR_ELSE(GColorShockingPink, GColorBlack), GColorWhite);

    radiobutton_window_push(radio_star);

    return true;
}

bool radio_button_test_deinit(void)
{
    return true;
}
