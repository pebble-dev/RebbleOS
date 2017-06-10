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
#include <stdbool.h>

// TODO     Make this dynamic. hacky 
#define NUM_APPS 3
#define MAX_APP_STR_LEN 32

typedef struct AppMessage
{
    uint8_t message_type_id;
    void *payload;
} AppMessage;

typedef struct ButtonMessage
{
    void *callback;
    void *clickref;
    void *context;
} ButtonMessage;

typedef struct TickMessage
{
    void *callback;
    struct tm* tick_time;
    TimeUnits tick_units;
} TickMessage;

typedef void (*AppMainHandler)(void);


typedef struct Version {
  uint8_t major;
  uint8_t minor;
} __attribute__((__packed__)) Version;

typedef struct Uuid {
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
    uint8_t b5;
    uint8_t b6;
    uint8_t b7;
    uint8_t b8;
    uint8_t b9;
    uint8_t b10;
    uint8_t b11;
    uint8_t b12;
    uint8_t b13;
    uint8_t b14;
    uint8_t b15;
} __attribute__((__packed__)) Uuid;
  
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
    uint8_t slot_id;
    uint32_t resource_address; // the address where we are keeping the resources for this app
    char *name;
    ApplicationHeader *header;
    AppMainHandler main; // A shortcut to main
    struct App *next;
} App;

typedef struct AppTypeHeader {
    char at;
    char address[8];
    char delim;
    char type[3];
} AppTypeHeader;


#define APP_BUTTON       0
#define APP_QUIT         1
#define APP_TICK         2

#define APP_TYPE_SYSTEM  0
#define APP_TYPE_FACE    1
#define APP_TYPE_APP     2


void appmanager_init(void);
void appmanager_post_button_message(ButtonMessage *bmessage);
void appmanager_post_tick_message(TickMessage *tmessage, BaseType_t *pxHigherPri);
void appmanager_app_start(char *name);
void appmanager_app_quit(void);
App *appmanager_get_app(char *app_name);
App *app_manager_get_apps_head();

void rbl_window_load_proc(void);
void app_event_loop(void);




