/* appmanager.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"
#include "api_func_symbols.h"

/*
 * Module TODO
 * 
 * Hook the flags up. These contain app type etc.
 * resource loading from apps
 * 
 */

static void _appmanager_flash_load_app_manifest();
App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal, uint8_t slot_id);
void _appmanager_app_thread(void *parameters);
void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);

TaskHandle_t _app_task_handle;
TaskHandle_t _app_thread_manager_task_handle;
static xQueueHandle _app_message_queue;
static xQueueHandle _app_thread_queue;
StaticTask_t _app_thread_manager_task;
StaticTask_t _app_task;

static App *_running_app;
static App *_app_manifest_head;

// simple doesn't have an include, so cheekily forward declare here
void simple_main(void);
void nivz_main(void);

#define APP_STACK_SIZE_IN_BYTES 96000
#define APP_THREAD_MANAGER_STACK_SIZE 1000
StackType_t _app_thread_manager_stack[APP_THREAD_MANAGER_STACK_SIZE];  // stack + heap for app (in words)

// We are abusing the stack area to store the app too. These need different memory sizes
union {
    uint8_t byte_buf[APP_STACK_SIZE_IN_BYTES]; // app memory
    StackType_t word_buf[0];
} _stack_addr;
    
    
void appmanager_init(void)
{
    // load the baked in 
    _appmanager_add_to_manifest(_appmanager_create_app("System", APP_TYPE_SYSTEM, systemapp_main, true, 0));
    _appmanager_add_to_manifest(_appmanager_create_app("Simple", APP_TYPE_FACE, simple_main, true, 0));
    _appmanager_add_to_manifest(_appmanager_create_app("NiVZ", APP_TYPE_FACE, nivz_main, true, 0));
    
    // now load the ones on flash
    _appmanager_flash_load_app_manifest();
    
    _app_message_queue = xQueueCreate(5, sizeof(struct AppMessage));
    _app_thread_queue = xQueueCreate(1, sizeof(struct AppMessage));    
   
    // set off using system
    appmanager_app_start("NiVZ");
    
    // create the thread
    _app_thread_manager_task_handle = xTaskCreateStatic(_appmanager_app_thread, "App", APP_THREAD_MANAGER_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5UL, _app_thread_manager_stack, &_app_thread_manager_task);
    
    printf("App Task Created!\n");
}


App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal, uint8_t slot_id)
{
    App *app = calloc(1, sizeof(App));
    if (app == NULL)
        return NULL;
    
    printf("CREATING\n");
    
    app->name = calloc(1, strlen(name) + 1);
    
    if (app->name == NULL)
        return NULL;
    
    strcpy(app->name, name);
    app->main = (void*)entry_point;
    app->type = type;
    app->header = NULL;
    app->next = NULL;
    app->slot_id = slot_id;
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
    for(uint8_t i = 0; i < 24; i++)
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
                // The task will die hard.
                // TODO: BAD! The task will never call the cleanup after loop!
                vTaskDelete(_app_task_handle);
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

    ApplicationHeader header;   // TODO change to malloc so we can free after load?
    char *app_name;
    AppMessage am;
        
    for( ;; )
    {
        printf("Pend RECV\n");
        // Sleep waiting for the go signal
        xQueueReceive(_app_thread_queue, &am, portMAX_DELAY);
        
        app_name = (char *)am.payload;
        
        printf("RECV App: %s\n", app_name);

        // clear the queue of any work from the previous app... such as an errant quit
        xQueueReset(_app_message_queue);
            
      
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
        
        if (!node->is_internal)
        {
            /*
             
             Heres what is going down. We are going to load the app header file from flash
             This contains sizes and offsets. Then we load the app itself.
             
             There is a symbol table in each Pebble app that needs to have the address 
             of _our_ symbol table poked into it. This allows the app to do a lookup of 
             a function id internally, check it in our rebble sym table, and return 
             a pointer address to to the function to jump to.
             
             We also have to make sure to init bss to 0 on fork otherwise it 
             may have garbage
             
             Then we set the Global Offset Table (GOT). Each Pebble app is compiled with
             Position Independant code (PIC) that means it can run from any address.
             This is usually used on dlls .so files and the like, and a Pebble app is no different
             It is the responsibility of the dynamic loader (us in this case) to get
             the GOT (which is at the end of the binary) and relocate all of the text symbols
             in the application binary.
             NOTE: once we have re-allocated, we don't need the GOT any more, so we can delete it
             
             For now, the statically allocated memory for the app task is also used
             to load the application into. The application needs uint8 size to execute 
             from, while the stack is uint32. The stack is therefore partitioned into 
                [app binary | stack++....--heap]
             
             The entry point given to the task (that spawns the app) is the beginning of the
             stack region, after the app binary. The relative bin and stack and then 
             unioned through as the right sizes to the BX and the SP.           
             
             We are leaning on Rtos here to actually spawn the app with it's own stack
             (cheap fork)  and make sure the (M)SP is set accordingly. 
             As a bonus we can manipulate this task in FreeRTOS directly through it's 
             TCB handle (suspen, kill, delete)
             
             Steps:
             * Find app on flash
             * Load app into lower stack
             * Set symbol table address
             * load relocs and reloc the GOT
             * zero BSS
             * fork
                 
             */
            uint32_t total_app_size = 0;
            flash_load_app_header(node->slot_id, &header);
        
            BssInfo bss = flash_get_bss(node->slot_id);

            // load the app from flash
            // and any reloc entries too.
            flash_load_app(node->slot_id, _stack_addr.byte_buf, header.app_size + (header.reloc_entries_count * 4));
            
            
            // re-allocate the GOT for -fPIC
            // Pebble has the GOT at the end of the app.
            // first get the GOT realloc table
            uint32_t *got = &_stack_addr.byte_buf[header.app_size];
            
            for (uint32_t i = 0; i < header.reloc_entries_count; i+=4)
            {
                // this screams cleanuo
                uint32_t got_val = (uint32_t)((unsigned char)(got[i]) << 24 |
                                            (unsigned char)(got[i + 1]) << 16 |
                                            (unsigned char)(got[i + 2]) << 8 |
                                            (unsigned char)(got[i + 3]));
                
                // global offset register in flash for the GOT
                uint32_t addr = _stack_addr.byte_buf + got_val;
                
                // get the existing value in the register
                uint32_t existing_offset_val = (uint32_t)((unsigned char)(_stack_addr.byte_buf[addr]) << 24 |
                                                        (unsigned char)(_stack_addr.byte_buf[addr + 1]) << 16 |
                                                        (unsigned char)(_stack_addr.byte_buf[addr + 2]) << 8 |
                                                        (unsigned char)(_stack_addr.byte_buf[addr + 3]));

                // The offset is offset in memory. i.e. existing offset in register + head of app
                uint32_t new_offset = existing_offset_val + _stack_addr.byte_buf;
                
                // Set the new value
                _stack_addr.byte_buf[addr]     =  (uint32_t)(new_offset) & 0xFF;
                _stack_addr.byte_buf[addr + 1] =  ((uint32_t)(new_offset) >> 8)  & 0xFF;
                _stack_addr.byte_buf[addr + 2] =  ((uint32_t)(new_offset) >> 16) & 0xFF;
                _stack_addr.byte_buf[addr + 3] =  ((uint32_t)(new_offset) >> 24) & 0xFF;
            }
            
            // init bss to 0
            uint32_t bss_size = bss.end_address - header.app_size;
            memset(header.app_size, 0, bss_size);
            
            // The app start will clear the re-alloc table values when they are used by stack

            // app size app size + bss + reloc
            total_app_size = header.app_size + bss_size;
            
            // load the address into the special register in the app. hopefully in a platformish independant way
            _stack_addr.byte_buf[header.sym_table_addr]     =     (uint32_t)(sym)         & 0xFF;
            _stack_addr.byte_buf[header.sym_table_addr + 1] =     ((uint32_t)(sym) >> 8)  & 0xFF;
            _stack_addr.byte_buf[header.sym_table_addr + 2] =     ((uint32_t)(sym) >> 16) & 0xFF;
            _stack_addr.byte_buf[header.sym_table_addr + 3] =     ((uint32_t)(sym) >> 24) & 0xFF;
            
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

            printf("A 0x%x\n", *(_stack_addr.byte_buf + header.sym_table_addr));
            printf("A 0x%x\n", _stack_addr.byte_buf[header.offset]);
            printf("A 0x%x\n", _stack_addr.byte_buf[header.offset +1]);
            printf("A 0x%x\n", _stack_addr.byte_buf[header.offset+2]);
 
            
            // Let this guy do the heavy lifting!
            _app_task_handle = xTaskCreateStatic(&_stack_addr.byte_buf[header.offset], "dynapp", (APP_STACK_SIZE_IN_BYTES - total_app_size) / sizeof(StackType_t), NULL, tskIDLE_PRIORITY + 5UL, &_stack_addr.word_buf[header.offset], &_app_task);
        }
        else
        {
            // start the app. this will block if the app is written to call
            // the main app_event_loop.
            // The main loop work is deferred to the app until it quits
            // 
            // actually spawn a static task here?
            ((AppMainHandler)(_running_app->main))();
        }
        // we unblocked. It looks like the app quit
        // around we go again
    }
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
    while(1);
}

void p_n_grect_standardize(n_GRect r)
{
    n_grect_standardize(r);
}
