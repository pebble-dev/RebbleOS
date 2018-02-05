/* appmanager_app_runloop.c
 * Common routines for managing apps/workers/threads
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"

/*
 * Start an application by name
  * The message contains the app name
 */
void appmanager_app_start(char *name)
{
    AppMessage am = (AppMessage) {
        .message_type_id = THREAD_MANAGER_APP_LOAD,
        .thread_id = AppThreadMainApp,
        .payload = name
    };
    appmanager_post_generic_thread_message(&am, 100);
}

void appmanager_app_quit(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_QUIT,
        .payload = NULL
    };
    appmanager_post_generic_app_message(&am, 10);
}

void appmanager_post_button_message(ButtonMessage *bmessage)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_BUTTON,
        .payload = (void *)bmessage
    };
    appmanager_post_generic_app_message(&am, 10);
}

void appmanager_post_draw_message(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_DRAW
    };
    appmanager_post_generic_app_message(&am, 10);
}
