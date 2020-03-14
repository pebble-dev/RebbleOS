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

#define MAX_APP_STR_LEN 32

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

typedef void (*AppMainHandler)(void);


typedef struct Version {
  uint8_t major;
  uint8_t minor;
} __attribute__((__packed__)) Version;

typedef struct ApplicationHeader {
    char header[8];                   // PBLAPP
    Version header_version;           // version of this header
    Version sdk_version;              // sdk it was compiled against it seems
    Version app_version;              // app version
    uint16_t app_size;                // size of app binary + app header (but not reloc)
    uint32_t offset;                  // beginning of the app binary
    uint32_t crc;                     // data's crc?
    char name[MAX_APP_STR_LEN];
    char company[MAX_APP_STR_LEN];
    uint32_t icon_resource_id;
    uint32_t sym_table_addr;          // The system will poke the sdk's symbol table address into this field on load
    uint32_t flags;
    uint32_t reloc_entries_count;     // reloc list count
    Uuid uuid;
    uint32_t resource_crc;
    uint32_t resource_timestamp;
    uint16_t virtual_size;            // The total amount of memory used by the process (.text + .data + .bss)
} __attribute__((__packed__)) ApplicationHeader;



typedef struct App {
    uint8_t type; // this will be in flags I presume <-- it is. TODO. Hook flags up
    bool is_internal; // is the app baked into flash
    struct file *app_file;
    struct file *resource_file; // the file where we are keeping the resources for this app
    char *name;
    Uuid uuid;
    ApplicationHeader *header;
    AppMainHandler main; // A shortcut to main
    list_node node; 
} App;

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

/* Are we loading? */
typedef enum AppThreadState {
    AppThreadUnloaded,
    AppThreadLoading,
    AppThreadLoaded,
    AppThreadRunloop,
    AppThreadUnloading,
} AppThreadState;

/* We have App
 * we want an array of running threads
 * These will all be running at the same time
 * Here are the types
 */
typedef enum AppThreadType {
    AppThreadMainApp,
    AppThreadWorker,
    AppThreadOverlay,
    MAX_APP_THREADS
} AppThreadType;


#define THREAD_MANAGER_APP_LOAD       0
#define THREAD_MANAGER_APP_LOAD_UUID  1
#define THREAD_MANAGER_APP_QUIT_CLEAN 2

/* This struct hold all information about the task that is executing
 * There are many runing apps, such as main app, worker or background.
 */
typedef struct app_running_thread_t {
    AppThreadType thread_type;
    App *app;
    AppThreadState status;
    void *thread_entry;
    TickType_t shutdown_at_tick;
    const char *thread_name;    
    uint8_t thread_priority;
    TaskHandle_t task_handle;
    StaticTask_t static_task;
    size_t stack_size;
    size_t heap_size;
    StackType_t *stack;
    uint8_t *heap;
    struct CoreTimer *timer_head;
    qarena_t *arena;
    struct n_GContext *graphics_context;
} app_running_thread;

/* in appmanager.c */
uint8_t appmanager_init(void);
void appmanager_timer_add(CoreTimer *timer);
void appmanager_timer_remove(CoreTimer *timer);
void app_event_loop(void);
bool appmanager_post_generic_thread_message(AppMessage *am, TickType_t timeout);
app_running_thread *appmanager_get_current_thread(void);
App *appmanager_get_current_app(void);
bool appmanager_is_thread_system(void);
bool appmanager_is_thread_worker(void);
bool appmanager_is_thread_app(void);
bool appmanager_is_thread_overlay(void);
void appmanager_load_app(app_running_thread *thread, ApplicationHeader *header);
void appmanager_execute_app(app_running_thread *thread, uint32_t total_app_size);
app_running_thread *appmanager_get_thread(AppThreadType type);
AppThreadType appmanager_get_thread_type(void);

/* in appmanager_app_runloop.c */
void appmanager_app_runloop_init(void);
void appmanager_app_main_entry(void);
list_head *app_manager_get_apps_head();
void appmanager_post_button_message(ButtonMessage *bmessage);
void appmanager_post_draw_message(uint8_t force);
void appmanager_post_draw_display_message(uint8_t *draw_to_display);
bool appmanager_post_event_message(uint16_t protocol_id, void *message, DestroyEventProc destroy_callback);
void appmanager_post_window_load_click_config(struct Window *window);

void appmanager_app_start(char *name);
void appmanager_app_quit(void);
void appmanager_app_display_done(void);
bool appmanager_is_app_shutting_down(void);

bool appmanager_post_generic_app_message(AppMessage *am, TickType_t timeout);
void appmanager_timer_expired(app_running_thread *thread);
TickType_t appmanager_timer_get_next_expiry(app_running_thread *thread);
/* in appmanager_app.c */
App *appmanager_get_app(char *app_name);
void appmanager_app_loader_init(void);

void rocky_event_loop_with_resource(uint16_t resource_id);

void timer_init(void);

typedef void* ClickRecognizerRef;
void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context);
App *appmanager_get_app_by_uuid(Uuid *uuid);
