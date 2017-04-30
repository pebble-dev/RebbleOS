/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"
#include "api_func_symbols.h"

static void _appmanager_flash_load_app_manifest();
App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal, uint8_t slot_id);
void _appmanager_app_thread(void *parameters);
void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);

TaskHandle_t _app_task_handle;
static xQueueHandle _app_message_queue;
static xQueueHandle _app_thread_queue;
StaticTask_t _app_task;

static App *_running_app;
static App *_app_manifest_head;

// simple doesn't have an include, so cheekily forward declare here
void simple_main(void);
void nivz_main(void);

#define APP_STACK_SIZE 20000

StackType_t _app_stack[APP_STACK_SIZE];  // stack + heap for app (in words)

void appmanager_init(void)
{
    // load the baked in 
    _appmanager_add_to_manifest(_appmanager_create_app("System", APP_TYPE_SYSTEM, systemapp_main, true, 0));
    _appmanager_add_to_manifest(_appmanager_create_app("Simple", APP_TYPE_FACE, simple_main, true, 0));
    _appmanager_add_to_manifest(_appmanager_create_app("NiVZ", APP_TYPE_FACE, nivz_main, true, 0));
    
    watchdog_reset();
    // now load the ones on flash
    _appmanager_flash_load_app_manifest();
    watchdog_reset();
    
    _app_message_queue = xQueueCreate(5, sizeof(struct AppMessage));
    _app_thread_queue = xQueueCreate(1, sizeof(struct AppMessage));
    
    // create the thread
    _app_task_handle = xTaskCreateStatic(_appmanager_app_thread, "App", APP_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5UL, _app_stack, &_app_task);
    
    printf("App Task Created!\n");
}


App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal, uint8_t slot_id)
{
    App *app = calloc(1, sizeof(App));
    if (app == NULL)
        return NULL;
    
    app->name = calloc(1, strlen(name) + 1);
    
    if (app->name == NULL)
        return NULL;
    
    strcpy(app->name, name);
    app->main = (void*)entry_point;
    app->type = type;
    app->header = NULL;
    app->next = NULL;
    app->slot_id = 0;
    app->is_internal = is_internal;
    
    return app;
}


/*
 * Load the list of apps and faces from flash
 */
void _appmanager_flash_load_app_manifest(void)
{
    ApplicationHeader header;
    
    // super cheesy
    // 8 app slots
    for(uint8_t i = 0; i < 8; i++)
    {
        flash_load_app_header(i, &header);
        
        // sanity check the hell out of this to make sure it's a real app
        if (!strncmp(header.header, "PBLAPP", 6))
        {
            // it's real... so far. Lets crc check to make sure
            // TODO
            // crc32....(header.header)
            printf("VALID App Found %s\n", header.name);
            // main gets set later
            _appmanager_add_to_manifest(_appmanager_create_app(header.name, APP_TYPE_FACE, NULL, false, i));
        }
    }
}


void _appmanager_add_to_manifest(App *app)
{  
    if (_app_manifest_head == NULL)
    {
        _app_manifest_head = app;
        return;
    }
    
    App *child = _app_manifest_head;
    
    // now find the last node
    while(child->next)
        child = child->next;
    
    // link the node to the last child
    child->next = app;
}


void appmanager_app_start(char *name)
{
    AppMessage am;
    
    // Kill the current app. This will send a clean terminate signal
    appmanager_app_quit();
    
    am.payload = name;
    
    // we are setup now for main.
    // signal go to the thread
    xQueueSendToBack(_app_thread_queue, &am, (TickType_t)100);
}

void appmanager_app_quit(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_QUIT,
        .payload = NULL
    };
    xQueueSendToBack(_app_message_queue, &am, (TickType_t)10);
}

void appmanager_post_button_message(ButtonMessage *bmessage)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_BUTTON,
        .payload = (void *)bmessage
    };
    xQueueSendToBack(_app_message_queue, &am, (TickType_t)10);
}

void appmanager_post_tick_message(TickMessage *tmessage, BaseType_t *pxHigherPri)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_TICK,
        .payload = (void *)tmessage
    };
    // Note the from ISR. The tic comes direct to the app event handler
    xQueueSendToBackFromISR(_app_message_queue, &am, pxHigherPri);
}

void app_event_loop(void)
{
    uint32_t xMaxBlockTime = 1000 / portTICK_RATE_MS;
    AppMessage data;
    
    printf("LOOP\n");
    
    // we assume they are configured now
    rbl_window_load_proc();
    rbl_window_load_click_config();
    
    // Install our own handler to hijack the long back press
    //window_long_click_subscribe(BUTTON_ID_BACK, 1100, back_long_click_handler, back_long_click_release_handler);
    
    // TODO move to using local running_app variable to make atomic
    window_single_click_subscribe(BUTTON_ID_SELECT, app_select_single_click_handler);
    // hook the return from menu if we are a system app
    if (_running_app->type == APP_TYPE_SYSTEM)
        window_single_click_subscribe(BUTTON_ID_BACK, back_long_click_handler);
    
    // redraw
    window_dirty(true);
    
    // block forever
    for ( ;; )
    {
        // we are inside the apps main loop event handler now
        if (xQueueReceive(_app_message_queue, &data, xMaxBlockTime))
        {
            if (data.message_type_id == APP_BUTTON)
            {
                // execute the button's callback
                ButtonMessage *message = (ButtonMessage *)data.payload;
                ((ClickHandler)(message->callback))((ClickRecognizerRef)(message->clickref), message->context);
            }
            else if (data.message_type_id == APP_TICK)
            {
                // execute the timers's callback
                TickMessage *message = (TickMessage *)data.payload;
                
                ((TickHandler)(message->callback))(message->tick_time, (TimeUnits)message->tick_units);
            }
            else if (data.message_type_id == APP_QUIT)
            {
                // remove all of the clck handlers
                button_unsubscribe_all();
                // remove the ticktimer service handler and stop it
                rebble_time_service_unsubscribe();

                printf("ev quit\n");
                // app was quit, break out of this loop into the main handler
                break;
            }
        }
    }
    // the app itself will quit now
}

/*
 * A task to run an application
 */
void _appmanager_app_thread(void *parms)
{
    printf("APP THREAD\n");
    uint8_t *running_app_program_memory = NULL;
    ApplicationHeader header;   // TODO change to malloc so we can free after load
    char *app_name;
    AppMessage am;
    
    // set off using system
    appmanager_app_start("System");
    
    
    for( ;; )
    {
        // Sleep waiting for the go signal
        xQueueReceive(_app_thread_queue, &am, portMAX_DELAY);
        
        app_name = (char *)am.payload;
        
        printf("RECV App: %s\n", app_name);

        // clear the queue of any work from the previous app... such as an errant quit
        xQueueReset(_app_message_queue);
            
        // kill the current app
    //     appmanager_app_quit();
        
        // TODO reset clicks

        
        if (_app_manifest_head == NULL)
        {
            printf("Bad, no apps\n");
            return;
        }
        
        // find the app
        App *node = _app_manifest_head;
        
        // now find the last node
        while(node)
        {
            if (!strncmp(node->name, (char *)app_name, strlen(node->name)))
            {
                // match!
                break;
            }
            
            if (node->next == NULL)
            {
                printf("No app found!\n");
                return;
            }
            node = node->next;
        }

        // it's the one
        _running_app = node;
        
        flash_load_app_header(node->slot_id, &header);
        
        if (!node->is_internal)
        {
            
            BssInfo bss = flash_get_bss(node->slot_id);
            
            // allocate the buffer for the memory to go into
            printf("WILL ALLOC %d %d\n", header.app_size + bss.size, bss.size);
            running_app_program_memory = calloc(1, header.app_size + bss.size);
            
            // load the app from flash
            flash_load_app(node->slot_id, running_app_program_memory, header.app_size);

            //init bss to 0
            uint32_t bss_start = bss.end_address - bss.size;
            memset(bss_start, 0, bss.size);

            // load the address into the special register in the app. hopefully in a platformish independant way
            running_app_program_memory[header.sym_table_addr]     =     (uint32_t)(sym)         & 0xFF;
            running_app_program_memory[header.sym_table_addr + 1] =     ((uint32_t)(sym) >> 8)  & 0xFF;
            running_app_program_memory[header.sym_table_addr + 2] =     ((uint32_t)(sym) >> 16) & 0xFF;
            running_app_program_memory[header.sym_table_addr + 3] =     ((uint32_t)(sym) >> 24) & 0xFF;
            
            printf("H:    %s\n", header.header);
            printf("SDKv: %d.%d\n", header.sdk_version.major, header.sdk_version.minor);
            printf("Appv: %d.%d\n", header.app_version.major, header.app_version.minor);
            printf("AppSz:%d\n", header.app_size);
            printf("AppOf:0x%x\n", header.offset);
            printf("AppCr:%d\n", header.crc);
            printf("Name: %s\n", header.name);
            printf("Cmpy: %s\n", header.company);
            printf("Icon: %d\n", header.icon_resource_id);
            
            printf("Sym:  0x%x\n", header.sym_table_addr);
            printf("Flags:%d\n", header.flags);
            printf("Reloc:%d\n", header.reloc_entries_count);

            // set the initial offset pointer
            // XXX NOTE -2 on the offset for some reason. TODO investigate
            printf("A 0x%x\n", running_app_program_memory[header.offset -2]);
            _running_app->main = &running_app_program_memory[header.offset - 2];
            printf("New:  0x%x 0x%x\n", running_app_program_memory, _running_app->main);
            printf("Mem:  0x%x 0x%x\n", (*_running_app->main), _running_app->main);
        }
        
        // start the app. this will block if the app is written to call
        // the main app_event_loop.
        // The main loop work is deferred to the app until it quits
        ((AppMainHandler)(_running_app->main))();
        
        // we unblocked. It looks like the app quit
        // around we go again
        if (running_app_program_memory != NULL)
            free(running_app_program_memory);
        
        running_app_program_memory = NULL;
    }

    vTaskDelete(_app_task_handle);
}

void back_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(_running_app->type)
    {
        case APP_TYPE_FACE:
            printf("TODO: Quiet time\n");
            break;
        case APP_TYPE_SYSTEM:
            // quit the app
            appmanager_app_start("Simple");
            break;
    }
}

void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("Long Back Rel\n");
}

void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(_running_app->type)
    {
        case APP_TYPE_FACE:
            appmanager_app_start("System");
            break;
        case APP_TYPE_SYSTEM:
            menu_select();
            break;
    }
}

App *app_manager_get_apps_head()
{
    return _app_manifest_head;
}




void test() //uint8_t lvl, const char *fmt, ...)
{
    printf("hello\n");
}

void test1(uint8_t lvl, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}

void api_unimpl()
{
    printf("UNK\n");
}

void p_n_grect_standardize(n_GRect r)
{
    n_grect_standardize(r);
}
