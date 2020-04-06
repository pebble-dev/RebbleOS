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

/*
 * Start an application by name
  * The message contains the app name
 */
void appmanager_app_start(char *name)
{
    App * app = appmanager_get_app_by_name(name);
    assert(app);
    appmanager_app_start_by_uuid(&app->uuid);
}

void appmanager_app_start_by_uuid(Uuid *uuid)
{
    /* get app by uuid */
    App * app = appmanager_get_app_by_uuid(uuid);   
    assert(app);
    AppMessage am = (AppMessage) {
        .command = THREAD_MANAGER_APP_LOAD_ID,
        .thread_id = AppThreadMainApp,
        .data = (void *)(uint32_t)app->id
    };
    appmanager_post_generic_thread_message(&am, 100);
}


void appmanager_app_quit_request(void)
{
    AppMessage am = (AppMessage) {
        .command = THREAD_MANAGER_APP_QUIT_REQUEST,
        .thread_id = AppThreadMainApp,
    };
    appmanager_post_generic_thread_message(&am, 10);
}

void appmanager_app_quit(void)
{
    AppMessage am = (AppMessage) {
        .command = AppMessageQuit,
        .data = NULL
    };
    appmanager_post_generic_app_message(&am, 10);
}


void appmanager_app_heartbeat(void)
{
    app_running_thread *_thread = appmanager_get_current_thread();
    AppMessage am = (AppMessage) {
        .thread_id = _thread->thread_type,
        .command = THREAD_MANAGER_APP_HEARTBEAT,
    };
    appmanager_post_generic_thread_message(&am, 100);
}

void appmanager_app_quit_done(void)
{
    app_running_thread *_thread = appmanager_get_current_thread();
    AppMessage am = (AppMessage) {
        .thread_id = _thread->thread_type,
        .command = THREAD_MANAGER_APP_TEARDOWN,
    };
    appmanager_post_generic_thread_message(&am, 100);
}

void appmanager_app_download_complete(void)
{
    AppMessage am = (AppMessage) {
        .command = THREAD_MANAGER_APP_DOWNLOAD_COMPLETE,
        .data = NULL
    };
    appmanager_post_generic_thread_message(&am, 10);
}

void appmanager_post_button_message(ButtonMessage *bmessage)
{
//     /* we post to overlay first, app thread gets it relayed */
//     overlay_window_post_button_message(bmessage);
    AppMessage am = (AppMessage) {
        .command = AppMessageButton,
        .data = (void *)bmessage
    };
    appmanager_post_generic_app_message(&am, 100);
}


void appmanager_post_draw_update(uint8_t status)
{
    app_running_thread *_thread = appmanager_get_thread(AppThreadMainApp);

    AppMessage am = (AppMessage) {
        .command = THREAD_MANAGER_APP_DRAW,
        .data = (void*)(uint32_t)status
    };

    appmanager_post_generic_thread_message(&am, 0);
}

void appmanager_post_draw_message(uint8_t force)
{
    appmanager_post_draw_update(0);
}

void appmanager_post_draw_app_message(uint8_t force)
{
    app_running_thread *_thread = appmanager_get_thread(AppThreadMainApp);
        
    AppMessage am = (AppMessage) {
        .command = AppMessageDraw,
        .data = (void*)(uint32_t)force
    };
    appmanager_post_generic_app_message(&am, 0);
}

/* Send a message to a running app. This could be an event, such as "hey data arrived" or
 * some other request, such as IPC between worker and main threads */
bool appmanager_post_event_message(uint16_t protocol_id, void *message, DestroyEventProc destroy_callback)
{
    AppMessage am = (AppMessage) {
        .command = AppMessageEvent,
        .subcommand = protocol_id,
        .data = message,
        .context = destroy_callback
    };
    app_running_thread *_thread = appmanager_get_thread(AppThreadMainApp);
    
    /* No runloop? defer to the next app */
    if (_thread->status != AppThreadRunloop)
    {
        overlay_window_post_event(protocol_id, message, destroy_callback);
        return true;
    }
    return appmanager_post_generic_app_message(&am, 20);
}

void appmanager_post_window_load_click_config(Window *window)
{
    AppMessage am = (AppMessage) {
        .command = AppMessageLoadClickConfig,
        .data = (void *)window,
    };
    appmanager_post_generic_app_message(&am, 10);
}
