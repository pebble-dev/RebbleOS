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

static void checkbox_complete_callback(bool* s_selections, void *context) {

    for(int i = 0; i < 6; i++) {
      APP_LOG("test",APP_LOG_LEVEL_DEBUG,"Option %d was %s", i, (s_selections[i] ? "selected" : "not selected"));
    }

    checkbox_window_pop((CheckboxWindow*)context, true);

}

bool checkbox_test_exec(void)
{
    
    checkers = checkbox_window_create(6, (CheckboxWindowCallbacks){
        .checkbox_complete = checkbox_complete_callback
    });
    
    checkbox_add_selection(checkers, "When this");
    checkbox_add_selection(checkers, "baby hits");
    checkbox_add_selection(checkers, "88 MPH you're");
    checkbox_add_selection(checkers, "going to see");
    checkbox_add_selection(checkers, "some serious");
    checkbox_add_selection(checkers, "stuff!");


    checkbox_set_selection_colors(checkers, PBL_IF_COLOR_ELSE(GColorPurple, GColorBlack), GColorWhite);

    checkbox_window_push(checkers, true);

    return true;
}

bool checkbox_test_deinit(void)
{
    return true;
}
