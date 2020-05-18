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

static void radiobutton_complete_callback(int current_selection, void *context) {

    APP_LOG("test",APP_LOG_LEVEL_DEBUG,"Submitted choice %d", current_selection);
    radiobutton_window_pop((RadiobuttonWindow*)context, true);

}

bool radio_button_test_exec(void)
{

    radio_star = radiobutton_window_create(5, (RadiobuttonWindowCallbacks){
        .radiobutton_complete = radiobutton_complete_callback
    });

    radiobutton_add_selection(radio_star, "Well here we");
    radiobutton_add_selection(radio_star, "are again");
    radiobutton_add_selection(radio_star, "It's always");
    radiobutton_add_selection(radio_star, "such a pleasure");
    radiobutton_add_selection(radio_star, "(no it's not)");

    radiobutton_set_selection_colors(radio_star, PBL_IF_COLOR_ELSE(GColorShockingPink, GColorBlack), GColorWhite);

    radiobutton_window_push(radio_star, true);

    return true;
}

bool radio_button_test_deinit(void)
{
    return true;
}
