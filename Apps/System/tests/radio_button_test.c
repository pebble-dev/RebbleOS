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
    /*set_radio_selection_colors(PBL_IF_COLOR_ELSE(GColorRed, GColorBlack), GColorWhite);

    add_radio_selection("Well here we");
    add_radio_selection("are again");
    add_radio_selection("It's always");
    add_radio_selection("such a pleasure");
    add_radio_selection("(No it's not...)");*/

    radiobutton_add_selection(radio_star, "Well here we");
    radiobutton_add_selection(radio_star, "are again");
    radiobutton_add_selection(radio_star, "It's always");
    radiobutton_add_selection(radio_star, "such a pleasure");

    radiobutton_window_push(radio_star);

    return true;
}

bool radio_button_test_deinit(void)
{
    return true;
}
