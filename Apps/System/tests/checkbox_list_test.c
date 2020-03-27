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
    checkbox_add_selection(checkers, "When this");
    checkbox_add_selection(checkers, "baby hits");
    checkbox_add_selection(checkers, "88 MPH you're");
    checkbox_add_selection(checkers, "going to see");

    set_checkbox_selection_colors(checkers, PBL_IF_COLOR_ELSE(GColorPurple, GColorBlack), GColorWhite);

    checkbox_window_push(checkers);

    return true;
}

bool checkbox_test_deinit(void)
{
    return true;
}
