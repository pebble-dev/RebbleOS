/* appmanager_app_runloop.c
 * Common routines for managing apps/workers/threads
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "overlay_manager.h"
#include "protocol.h"
#include "protocol_music.h"
#include "event_service.h"
/*
 * Start an application by name
  * The message contains the app name
 */
void appmanager_app_start(char *name)
{
    AppMessage am = (AppMessage) {
        .command = THREAD_MANAGER_APP_LOAD,
        .thread_id = AppThreadMainApp,
        .data = name
    };
    appmanager_post_generic_thread_message(&am, 100);
}

void appmanager_app_quit(void)
{
    AppMessage am = (AppMessage) {
        .command = AppMessageQuit,
        .data = NULL
    };
    appmanager_post_generic_app_message(&am, 10);
}

void appmanager_post_button_message(ButtonMessage *bmessage)
{
    /* we post to overlay first, app thread gets it relayed */
    overlay_window_post_button_message(bmessage);
}

void appmanager_post_draw_message(uint8_t force)
{
    app_running_thread *_thread = appmanager_get_thread(AppThreadMainApp);
    if (_thread->status != AppThreadRunloop)
        return;

    AppMessage am = (AppMessage) {
        .command = AppMessageDraw,
        .data = (void*)(uint32_t)force
    };

    Window *wind = window_stack_get_top_window();
    Window *owind = overlay_window_stack_get_top_window();

    if (force || 
            ((wind && wind->is_render_scheduled) ||
             (owind && owind->is_render_scheduled)))
    {
        appmanager_post_generic_app_message(&am, 0);
    }
}

/* Send a message to a running app. This could be an event, such as "hey data arrived" or
 * some other request, such as IPC between worker and main threads */
void appmanager_post_event_message(uint16_t protocol_id, void *message, DestroyEventProc destroy_callback)
{
        AppMessage am = (AppMessage) {
        .command = AppMessageEvent,
        .subcommand = protocol_id,
        .data = message,
        .context = destroy_callback
    };
    appmanager_post_generic_app_message(&am, 10);
}
