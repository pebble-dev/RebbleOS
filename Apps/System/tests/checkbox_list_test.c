/* checkbox_list_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "checkbox_window.h"
#include "test_defs.h"

bool checkbox_test_init(Window *window)
{
    return true;
}

bool checkbox_test_exec(void)
{

    set_checkbox_selection_colors(PBL_IF_COLOR_ELSE(GColorRed, GColorBlack), GColorWhite);

    add_checkbox_selection("Good evening");
    add_checkbox_selection("Twitter,");
    add_checkbox_selection("It's ya boy");
    add_checkbox_selection("Eatdatpussy445");

    checkbox_window_push();

    return true;
}

bool checkbox_test_deinit(void)
{
    return true;
}
