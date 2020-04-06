#pragma once
/* appmanager.h
 * Routines for managing the loading of applications dynamically
 * Each app is loaded with its own stack and heap.
 *   (https://github.com/pebble-dev)
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "rebble_time.h"
#include "fs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "qalloc.h"
#include "node_list.h"
#include <stdbool.h>
#include "uuid.h"
#include "event_service.h"

struct Window;

typedef struct CoreTimer
{
    TickType_t when; /* ticks when this should fire, in ticks since boot */
    void (*callback)(struct CoreTimer *); /* always called back on the app thread */
    struct CoreTimer *next;
} CoreTimer;

typedef struct AppMessage
{
    uint8_t thread_id;
    uint8_t command;
    uint8_t subcommand;
    void *data;
    void *context;
} AppMessage;

typedef struct ButtonMessage
{
    void *callback;
    void *clickref;
    void *context;
} ButtonMessage;


typedef struct AppTypeHeader {
    char at;
    char address[8];
    char delim;
    char type[3];
} AppTypeHeader;

/* Allowed command types when posted to the app runloop queue */
typedef enum {
    AppMessageButton,
    AppMessageQuit,
    AppMessageTick,
    AppMessageDraw,
    AppMessageEvent,
    AppMessageLoadClickConfig,
} AppMesssageType;

enum {
    AppTypeSystem,
    AppTypeWatchface,
    AppTypeApp
};

/* Running App stuff */

#define THREAD_MANAGER_APP_LOAD_ID           0
#define THREAD_MANAGER_APP_QUIT_REQUEST      1
#define THREAD_MANAGER_APP_TEARDOWN          2
#define THREAD_MANAGER_APP_DOWNLOAD_COMPLETE 3
#define THREAD_MANAGER_APP_DOWNLOAD_PROGRESS 4
#define THREAD_MANAGER_APP_DRAW              5
#define THREAD_MANAGER_APP_HEARTBEAT         6

/* in appmanager.c */
uint8_t appmanager_init(void);
void appmanager_timer_add(CoreTimer *timer, app_running_thread *thread);
void appmanager_timer_remove(CoreTimer *timer, app_running_thread *thread);
void app_event_loop(void);
bool appmanager_post_generic_thread_message(AppMessage *am, TickType_t timeout);
app_running_thread *appmanager_get_current_thread(void);
App *appmanager_get_current_app(void);
bool appmanager_is_thread_system(void);
bool appmanager_is_thread_worker(void);
bool appmanager_is_thread_app(void);
bool appmanager_is_thread_overlay(void);
int appmanager_load_app(app_running_thread *thread, ApplicationHeader *header);
void appmanager_execute_app(app_running_thread *thread, uint32_t total_app_size);
app_running_thread *appmanager_get_thread(AppThreadType type);
app_running_thread *appmanager_get_threads(void);
AppThreadType appmanager_get_thread_type(void);

/* in appmanager_app_runloop.c */
void appmanager_app_runloop_init(void);
void appmanager_app_main_entry(void);
list_head *app_manager_get_apps_head();
void appmanager_post_button_message(ButtonMessage *bmessage);
void appmanager_post_draw_message(uint8_t force);
void appmanager_post_draw_update(uint8_t status);
bool appmanager_post_event_message(uint16_t protocol_id, void *message, DestroyEventProc destroy_callback);
void appmanager_post_window_load_click_config(struct Window *window);

void appmanager_app_start(char *name);
void appmanager_app_start_by_uuid(Uuid *uuid);
void appmanager_app_quit(void);
void appmanager_app_quit_request(void);
void appmanager_app_quit_done(void);
void appmanager_app_heartbeat(void);
void appmanager_app_download_complete(void);
void appmanager_app_display_done(void);
bool appmanager_is_app_shutting_down(void);
bool appmanager_is_app_running(void);

bool appmanager_post_generic_app_message(AppMessage *am, TickType_t timeout);
void appmanager_timer_expired(app_running_thread *thread);
TickType_t appmanager_timer_get_next_expiry(app_running_thread *thread);

/* in appmanager_app.c */
App *appmanager_get_app_by_name(char *app_name);
App *appmanager_get_app_by_id(uint32_t id);
App *appmanager_get_app_by_uuid(Uuid *uuid);
void appmanager_app_loader_init(void);
uint32_t appmanager_get_next_appid(void);
void appmanager_app_loader_init_n();
void rocky_event_loop_with_resource(uint16_t resource_id);

void timer_init(void);

typedef void* ClickRecognizerRef;
void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context);

void appmanager_post_draw_app_message(uint8_t force);
void appmanager_app_set_flag(App *app, uint8_t flag, bool value);
