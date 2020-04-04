#pragma once
#include "FreeRTOS.h"
#include "task.h"
#include "fs.h"
#include <stdbool.h>
#include "uuid.h"
#include "node_list.h"

#define MAX_APP_STR_LEN 32

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


/* Are we loading? */
typedef enum AppThreadState {
    AppThreadUnloaded,
    AppThreadLoading,
    AppThreadLoaded,
    AppThreadRunloop,
    AppThreadUnloading,
    AppThreadDownloading,
} AppThreadState;

typedef enum {
    ExecuteFromInternalFlash,
    AppFilePresent,
    ResourceFilePresent,
} AppFlags;


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
    uint8_t flags;
    struct file app_file;
    struct file resource_file; // the file where we are keeping the resources for this app
    char *name;
    uint32_t id;
    Uuid uuid;
    ApplicationHeader *header;
    AppMainHandler main; // A shortcut to main
    list_node node; 
} App;


/* This struct hold all information about the task that is executing
 * There are many runing apps, such as main app, worker or background.
 */
typedef struct app_running_thread_t {
    AppThreadType thread_type;
    App *app;
    AppThreadState status;
    void *thread_entry;
    TickType_t app_start_tick;
    TickType_t shutdown_at_tick;
    const char *thread_name;    
    uint8_t thread_priority;
    TaskHandle_t task_handle;
    StaticTask_t static_task;
    size_t stack_size;
    StackType_t *stack;
    struct CoreTimer *timer_head;
    struct mem_heap *heap;
    struct n_GContext *graphics_context;
} app_running_thread;
