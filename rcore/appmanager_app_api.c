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

void appmanager_post_draw_message(uint32_t timeout_ms)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_DRAW
    };
    appmanager_post_generic_app_message(&am, pdMS_TO_TICKS(timeout_ms));
}

void appmanager_post_draw_display_message(uint8_t *draw_to_display)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_DRAW_DONE,
        .payload = (void *)draw_to_display
    };
    appmanager_post_generic_app_message(&am, portMAX_DELAY);
}

void appmanager_app_display_done(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_DISPLAY_DONE,
        .payload = NULL
    };
    appmanager_post_generic_app_message(&am, 10);
}
