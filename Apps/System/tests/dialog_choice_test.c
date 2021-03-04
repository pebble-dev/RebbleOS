/* dialog_choice_test.c
 * RebbleOS
 * 
 * Author: Taylor E. <taylor@stanivision.com>.
 */

#include "librebble.h"
#include "dialog_choice_window.h"
#include "test_defs.h"

DialogChoiceWindow *dial;

bool dialog_choice_test_init(Window *window)
{
    return true;
}

static void dialog_choice_complete_callback(bool* s_selections, void *context) {

    APP_LOG("test",APP_LOG_LEVEL_DEBUG,"Option was %s", );

    dialog_choice_window_pop((DialogChoiceWindow*)context, true);

}

bool dialog_choice_test_exec(void)
{

    dial = dialogchoice_window_create( (DialogChoiceWindowCallbacks){
        .dialog_choice_complete = dialog_choice_complete_callback
    });

    dialogchoice_window_set_message(dial, "Set phasers to kill?");
    dialog_choice_window_push(dial, true);

    return true;
}

bool dialog_choice_test_deinit(void)
{
    return true;
}
