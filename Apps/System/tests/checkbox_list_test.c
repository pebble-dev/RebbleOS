/* checkbox_list_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "checkbox_window.h"
#include "test_defs.h"

static CheckboxWindow *checkers;

bool checkbox_test_init(Window *window)
{
    return true;
}

bool checkbox_test_exec(void)
{
    
    checkers = checkbox_window_create(4);
    checkbox_add_selection(checkers, "Good evening");
    checkbox_add_selection(checkers, "Twitter,");
    checkbox_add_selection(checkers, "It's ya boy");
    checkbox_add_selection(checkers, "Eatdatpussy445");

    set_checkbox_selection_colors(checkers, PBL_IF_COLOR_ELSE(GColorPurple, GColorBlack), GColorWhite);

    checkbox_window_push(checkers);

    return true;
}

bool checkbox_test_deinit(void)
{
    return true;
}
