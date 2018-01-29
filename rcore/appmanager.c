/* appmanager.c
 * Routines for managing the loading of applications dynamically
 * Each app is loaded with its own stack and heap.
 *   (https://github.com/pebble-dev)
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"
#include "test.h"
#include "notification.h"
#include "api_func_symbols.h"

/*
 * Module TODO
 * 
 * Hook the flags up. These contain app type etc.
 * break the exec into smaller parts
 * Move event loop out
 * 
 */

extern void appHeapInit(size_t, uint8_t*);
extern  GFont *fonts_load_custom_font(ResHandle*, struct file *file);

static void _appmanager_flash_load_app_manifest();
static App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal,
                                   const struct file *app_file, const struct file *resource_file);
static void _appmanager_app_thread(void *parameters);
static void _appmanager_add_to_manifest(App *app);

void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context);

static TaskHandle_t _app_task_handle;
static TaskHandle_t _app_thread_manager_task_handle;
static xQueueHandle _app_message_queue;
static xQueueHandle _app_thread_queue;
static StaticTask_t _app_thread_manager_task;
static StaticTask_t _app_task;

static App *_running_app;
static App *_app_manifest_head;

/* The manager thread needs only a small stack */
#define APP_THREAD_MANAGER_STACK_SIZE 300
StackType_t _app_thread_manager_stack[APP_THREAD_MANAGER_STACK_SIZE];  // stack + heap for app (in words)

// We are abusing the stack area to store the app too. These need different memory sizes
union {
    uint8_t byte_buf[MAX_APP_MEMORY_SIZE]; // app memory
    StackType_t word_buf[0];
} app_stack_heap;


// simple doesn't have an include, so cheekily forward declare here
void simple_main(void);
void nivz_main(void);

/*
 * Load any pre-existing apps into the manifest, search for any new ones and then start up
 */
void appmanager_init(void)
{
    struct file empty = { 0, 0, 0 }; // TODO: make files optional in `App` to avoid this
    
    // load the baked in 
    _appmanager_add_to_manifest(_appmanager_create_app("System", APP_TYPE_SYSTEM, systemapp_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Simple", APP_TYPE_FACE, simple_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("NiVZ", APP_TYPE_FACE, nivz_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Settings", APP_TYPE_SYSTEM, test_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Notification", APP_TYPE_SYSTEM, notif_main, true, &empty, &empty));
    _app_task_handle = NULL;
    
    // now load the ones on flash
    _appmanager_flash_load_app_manifest();
    
    _app_message_queue = xQueueCreate(5, sizeof(struct AppMessage));
    _app_thread_queue = xQueueCreate(1, sizeof(struct AppMessage));
   
    // set off using system
    //appmanager_app_start("91 Dub 4.0");
    appmanager_app_start("System");
    
    // create the task manager thread
    _app_thread_manager_task_handle = xTaskCreateStatic(_appmanager_app_thread, 
                                                        "App", 
                                                        APP_THREAD_MANAGER_STACK_SIZE, 
                                                        NULL, 
                                                        tskIDLE_PRIORITY + 5UL, 
                                                        _app_thread_manager_stack, 
                                                        &_app_thread_manager_task);
    
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App thread created");
}

/*
 * 
 * Generate an entry in the application manifest for each found app.
 * 
 */
static App *_appmanager_create_app(char *name, uint8_t type, void *entry_point, bool is_internal,
                                   const struct file *app_file, const struct file *resource_file)
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
    app->app_file = *app_file;
    app->resource_file = *resource_file;
    app->is_internal = is_internal;
    
    return app;
}

/* note that these flags are inverted */
#define APPDB_DBFLAGS_WRITTEN 1
#define APPDB_DBFLAGS_OVERWRITING 2
#define APPDB_DBFLAGS_DEAD 4

#define APPDB_IS_EOF(ent) (((ent).last_modified == 0xFFFFFFFF) && ((ent).hash == 0xFF) && ((ent).dbflags == 0x3F) && ((ent).key_length == 0x7F) && ((ent).value_length == 0x7FF))

struct appdb
{
    /* below is common to all settings DB entries */
    uint32_t last_modified;  //0x58F6AExx
    uint8_t hash;
    uint8_t dbflags:6;
    uint32_t key_length:7;
    uint32_t value_length:11;

    uint32_t application_id;
    
    Uuid app_uuid;  // 16 bytes
    uint32_t flags; /* pebble_process_info.h, PebbleProcessInfoFlags in the SDK */
    uint32_t icon;
    uint8_t app_version_major, app_version_minor;
    uint8_t sdk_version_major, sdk_version_minor;
    uint8_t app_face_bg_color, app_face_template_id;
    uint8_t app_name[32];
    uint8_t unk_arr_company[32];  // always blank
    uint8_t unk_arr[32]; // always blank
} __attribute__((__packed__));


/*
 * Load the list of apps and faces from flash
 * The app manifest is a list of all known applications we found in flash
 * We load all entries from `appdb` file.
 * TODO: appdb seems to have duplicates (in my case), maybe we should use `pmap`, but it was missing some entries for me
 */
void _appmanager_flash_load_app_manifest(void)
{
    struct file file;

    if (fs_find_file(&file, "appdb") < 0)
    {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "APPDB file not found");
        return;
    }

    char buffer[14];
    struct appdb appdb;
    struct fd fd;
    struct file app_file;
    struct file res_file;
    struct fd app_fd;
    ApplicationHeader header;

    fs_open(&fd, &file);

    // skipping 8 bytes for appdb file header
    fs_seek(&fd, 8, FS_SEEK_SET);
    for (int i = 0; i < file.size / sizeof(struct appdb); ++i) {
        if (fs_read(&fd, &appdb, sizeof(appdb)) != sizeof(appdb))
            break;

        if (APPDB_IS_EOF(appdb))
            break;

        if (appdb.dbflags & APPDB_DBFLAGS_WRITTEN) {
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file that is not written before eof, at index %d", i);
            continue;
        }
        
        if ((appdb.dbflags & APPDB_DBFLAGS_DEAD) == 0)
            continue;
        
        if ((appdb.dbflags & APPDB_DBFLAGS_OVERWRITING) == 0)
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file %08x is mid-overwrite; I feel nervous", appdb.application_id);

        if (appdb.application_id == 0xFFFFFFFFu) {
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file is written, but has no contents?");
            break;
        }
        
        snprintf(buffer, 14, "@%08lx/app", appdb.application_id);
        if (fs_find_file(&app_file, buffer) < 0)
            continue;

        snprintf(buffer, 14, "@%08lx/res", appdb.application_id);
        if (fs_find_file(&res_file, buffer) < 0)
            continue;

        fs_open(&app_fd, &app_file);

        if (fs_read(&app_fd, &header, sizeof(ApplicationHeader)) != sizeof(ApplicationHeader))
            break;
       
        // sanity check the hell out of this to make sure it's a real app
        if (!strncmp(header.header, "PBLAPP", 6))
        {
            // it's real... so far. Lets crc check to make sure
            // TODO
            // crc32....(header.header)
            KERN_LOG("app", APP_LOG_LEVEL_INFO, "appdb: app \"%s\" found, flags %08x, icon %08x", header.name, appdb.flags, appdb.icon);

            // main gets set later
            _appmanager_add_to_manifest(_appmanager_create_app(header.name,
                                                               APP_TYPE_FACE,
                                                               NULL,
                                                               false,
                                                               &app_file,
                                                               &res_file));
        }
    }
}

/* App manifest is a linked list. Just slot it in */
static void _appmanager_add_to_manifest(App *app)
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

/*
 * Get an application by name. NULL if invalid
 */
App *appmanager_get_app(char *app_name)
{
    // find the app
    App *node = _app_manifest_head;
    
    // now find the matching
    while(node)
    {
        if (!strncmp(node->name, (char *)app_name, strlen(node->name)))
        {
            // match!
            return node;
        }

        node = node->next;
    }
    
    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "NO App Found %s", app_name);
    return NULL;
}


/*
 * Start an application by name
 * This will send a kill -9 to the current app and send a queued message
 * The message contains the app name
 */
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

void appmanager_post_draw_message(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_DRAW
    };
    xQueueSendToBack(_app_message_queue, &am, (TickType_t)10);
}

/* Always adds to the running app's queue.  Note that this is only
 * reasonable to do from the app thread: otherwise, you can race with the
 * check for the timer head.  */
void appmanager_timer_add(CoreTimer *timer)
{
    CoreTimer **tnext = &_running_app->timer_head;
    
    /* until either the next pointer is null (i.e., we have hit the end of
     * the list), or the thing that the next pointer points to is further in
     * the future than we are (i.e., we want to insert before the thing that
     * the next pointer points to)
     */
    while (*tnext && (timer->when > (*tnext)->when)) {
        tnext = &((*tnext)->next);
    }
    
    timer->next = *tnext;
    *tnext = timer;
}

void appmanager_timer_remove(CoreTimer *timer)
{
    CoreTimer **tnext = &_running_app->timer_head;
    
    while (*tnext) {
        if (*tnext == timer) {
            *tnext = timer->next;
            return;
        }
        tnext = &(*tnext)->next;
    }
    
    assert(!"appmanager_timer_remove did not find timer in list");
}

/*
 * 
 * Once an application is spawned, it calls into app_event_loop
 * This function is a busy loop, but with the benefit that it is also a task
 * In here we are the main event handler, for buttons quits etc etc.
 * 
 */
void app_event_loop(void)
{
    AppMessage data;
    
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App entered mainloop");
    
    // Do this before window load, that way they have a chance to override
    if (_running_app->type != APP_TYPE_FACE)
    {
        // Enables default closing of windows, and through that, apps
        window_single_click_subscribe(BUTTON_ID_BACK, app_back_single_click_handler);
    }
    
    window_configure();
    
    // Install our own handler to hijack the long back press
    //window_long_click_subscribe(BUTTON_ID_BACK, 1100, back_long_click_handler, back_long_click_release_handler);
    
    if (_running_app->type != APP_TYPE_SYSTEM)
    {
        // TODO move to using local running_app variable to make atomic
        window_single_click_subscribe(BUTTON_ID_SELECT, app_select_single_click_handler);
    }
    
    // hook the return from menu if we are a system app
//     if (_running_app->type == APP_TYPE_SYSTEM)
//         window_single_click_subscribe(BUTTON_ID_BACK, back_long_click_handler);
    
    // redraw
    window_draw();
    
    // block forever
    for ( ;; )
    {
        /* Is there something queued up to do?  If so, we have the potential to do it. */
        TickType_t next_timer;
        
        if (_running_app->timer_head) {
            TickType_t curtime = xTaskGetTickCount();
            if (curtime > _running_app->timer_head->when)
                next_timer = 0;
            else
                next_timer = _running_app->timer_head->when - curtime;
        } else {
            next_timer = portMAX_DELAY; /* Just block forever. */
        }
        
        // we are inside the apps main loop event handler now
        if (xQueueReceive(_app_message_queue, &data, next_timer))
        {
            /* We woke up for some kind of event that someone posted.  But what? */
            KERN_LOG("app", APP_LOG_LEVEL_INFO, "Queue Receive");
            if (data.message_type_id == APP_BUTTON)
            {
                // execute the button's callback
                ButtonMessage *message = (ButtonMessage *)data.payload;
                ((ClickHandler)(message->callback))((ClickRecognizerRef)(message->clickref), message->context);
            }
            else if (data.message_type_id == APP_QUIT)
            {
                // remove all of the clck handlers
                button_unsubscribe_all();
                // remove the ticktimer service handler and stop it
                tick_timer_service_unsubscribe();

                KERN_LOG("app", APP_LOG_LEVEL_INFO, "App Quit");
                // The task will die hard.
                // TODO: BAD! The task will never call the cleanup after loop!
                vTaskDelete(_app_task_handle);
                // app was quit, break out of this loop into the main handler
                break;
            }
            else if (data.message_type_id == APP_DRAW)
            {
                window_draw();
            }
        } else {
            /* We woke up because we hit a timer expiry.  Dequeue first,
             * then invoke -- otherwise someone else could insert themselves
             * at the head, and we would wrongfully dequeue them!  */
            CoreTimer *timer = _running_app->timer_head;
            assert(timer);
            
            KERN_LOG("app", APP_LOG_LEVEL_INFO, "woke up for a timer");

            _running_app->timer_head = timer->next;
            
            timer->callback(timer);
        }
    }
    // the app itself will quit now
}

/*
 * A task to run an application.
 * 
 * This task runs all the time and is a dynamic app loader and thread spawner
 * Once an app is loaded, it is handed off to a new task. 
 * The new task is created with a statically allocated array of MAX_APP_MEMORY_SIZE
 * This array is used as the heap and the stack.
 * refer to heap_app.c (for now, until the refactor) TODO
 */
static void _appmanager_app_thread(void *parms)
{
    ApplicationHeader header;   // TODO change to malloc so we can free after load?
    char *app_name;
    AppMessage am;
       
    for( ;; )
    {
        // Sleep waiting for the go signal. The app to start will be the parameter
        // generally, at this point we would have an app to run, but serves as a nice block for now
        // TODO There is actually no way to fully block an errant requet to load two apps.
        // We need to check state and quit the existing app properly
        xQueueReceive(_app_thread_queue, &am, portMAX_DELAY);
        
        app_name = (char *)am.payload;
        
        KERN_LOG("app", APP_LOG_LEVEL_INFO, "Starting app %s", app_name);

        // clear the queue of any work from the previous app... such as an errant quit
        xQueueReset(_app_message_queue);            
      
        // TODO reset clicks
        tick_timer_service_unsubscribe();
        
        if (_app_manifest_head == NULL)
        {
            KERN_LOG("app", APP_LOG_LEVEL_ERROR, "No Apps found!");
            assert(!"No Apps");
            return;
        }
        
        // find the app
        App *app = appmanager_get_app(app_name);
        
        if (app == NULL)
            continue;

        // it's the one
        _running_app = app;
        
        if (_app_task_handle != NULL) {
            vTaskDelete(_app_task_handle);
            _app_task_handle = NULL;
        }
        _running_app->timer_head = NULL;
        
        // If the app is running off RAM (i.e it's a PIC loaded app...) and not system, we need to patch it
        if (!app->is_internal)
        {
            /*
             
             Heres what is going down. We are going to load the app header file from flash
             This contains sizes and offsets. The app bin sits directly after the app header.
             Then we load the app itself as well as the header block (for future reference)
             
             There is a symbol table in each Pebble app that needs to have the address 
             of _our_ symbol table poked into it. 
             The symbol table is a big lookup table of pointer functions. When an app calls aa function
             such as window_create() it actually turns that into an integer id in the app.
             When the app calls the function, it does function => id => RebbleOS => id to function => call
             This allows the app to do a lookup of a function id internally, check it in our rebble sym table, and return a pointer address to to the function to jump to.
             
             We also have to take care of BSS section. This is defined int he app header for the size, 
             and sits directly after the app binary. We alocate enough room for the BSS, and zero it out
             BSS. is always zeroed.
             
             Then we set the Global Offset Table (GOT). Each Pebble app is compiled with
             Position Independant code (PIC) that means it can run from any address.
             This is usually used on dlls .so files and the like, and a Pebble app is no different
             It is the responsibility of the dynamic loader (us in this case) to get
             the GOT (which is at the end of the binary) and relocate all of the data symbols
             in the application binary.
             The got is a clever way of doing relative lookups in the app for "shared" data.
             
             http://grantcurell.com/2015/09/21/what-is-the-symbol-table-and-what-is-the-global-offset-table/
             
             In short, the app header defines how many relocations of memory we need to do, and where in the bin
             these relocation registers are. 
             The loader does this:
             load GOT from end of binary using header offset value. for each entry, 
             get the position in memory for our GOT lookup table
             The value in the got table inside the .DATA section is updated to add the base address
             of the executing app to the relative offset already stored in the register.
             It is usual in a shred lib to have a shared data section between all apps. The data is copied
             to a separate location for each instance. 
             In this case we aer using the .DATA section in place. No sharing. no clever. Not required.
             
             When thwe app executes, any variables stored in the global are looked up...
             int global_a;
             printf(global_a);
                > go to the GOT in the DATA section by relative address and get the address of global_a
                > go to the address of global_a in data section
                > this is the allocated memory for globals
                > retrieve value
             
             NOTE: once we have relocated, we don't need the loaded header GOT any more, so we can delete it
             in fact the bss section will zero over it.
             
             For now, the statically allocated memory for the app task is also used
             to load the application into. The application needs uint8 size to execute 
             from, while the stack is uint32. The stack is therefore partitioned into 
               fixed_memory_for_app[n] = [ app binary | GOT || BSS | heap++....  | ...stack ]
             
             The entry point given to the task (that spawns the app) is the beginning of the
             stack region, after the app binary. The relative bin and stack are then 
             unioned through as the right sizes to the BX and the SP.
             The app deals with 8 bit bytes where the staack is a word of uint32. To use the same array,
             the value is unioned sto save epic bitshifting
             
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
            struct fd fd;
            fs_open(&fd, &app->app_file);
            fs_read(&fd, &header, sizeof(ApplicationHeader));
        
            // load the app from flash
            // and any reloc entries too.
            fs_seek(&fd, 0, FS_SEEK_SET);
            fs_read(&fd, app_stack_heap.byte_buf, header.app_size + (header.reloc_entries_count * 4));
            
            
            // re-allocate the GOT for -fPIC
            // Pebble has the GOT at the end of the app.
            // first get the GOT realloc table
            uint32_t *got = &app_stack_heap.word_buf[header.app_size / 4];

            if (header.reloc_entries_count > 0)
            {                
                // go through all of the reloc entries and do the reloc dance
                for (uint32_t i = 0; i < header.reloc_entries_count; i++)
                {
                    // get the got out of the table.
                    // use that value to get the relative offset from the address
                    uint32_t existing = app_stack_heap.word_buf[got[i]/4];

                    // we are working in words
                    existing /= 4;
                    
                    // take the offset and add the apps base address
                    // We are doing some nasty things with pointers here, where we are getting the base address
                    // adding the offset in the register and forcing its new adderss back in
                    // here we are going to go through a uint pointer for type safety
                    uintptr_t wb = (uintptr_t)(app_stack_heap.word_buf + existing);
                    
                    // write it back to the register
                    app_stack_heap.word_buf[got[i]/4] = wb;
                }
            }
            
            // init bss to 0
            uint32_t bss_size = header.virtual_size - header.app_size;
            memset(&app_stack_heap.byte_buf[header.app_size], 0, bss_size);
            
            // app size app size + bss
            total_app_size = header.virtual_size;
            
            // load the address of our lookup table into the special register in the app. hopefully in a platformish independant way
            app_stack_heap.byte_buf[header.sym_table_addr]     =     (uint32_t)(sym)         & 0xFF;
            app_stack_heap.byte_buf[header.sym_table_addr + 1] =     ((uint32_t)(sym) >> 8)  & 0xFF;
            app_stack_heap.byte_buf[header.sym_table_addr + 2] =     ((uint32_t)(sym) >> 16) & 0xFF;
            app_stack_heap.byte_buf[header.sym_table_addr + 3] =     ((uint32_t)(sym) >> 24) & 0xFF;
            
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "App signature:");
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "H:    %s", header.header);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "SDKv: %d.%d", header.sdk_version.major, header.sdk_version.minor);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Appv: %d.%d", header.app_version.major, header.app_version.minor);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppSz:%x", header.app_size);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppOf:0x%x", header.offset);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppCr:%d", header.crc);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Name: %s", header.name);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Cmpy: %s", header.company);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Icon: %d", header.icon_resource_id);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Sym:  0x%x", header.sym_table_addr);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Flags:%d", header.flags);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Reloc:%d", header.reloc_entries_count);
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "VSize 0x%x", header.virtual_size);
             
            uint32_t stack_size = MAX_APP_STACK_SIZE;
            
            // Get the start point of the stack in the array
            uint32_t *stack_entry = &app_stack_heap.word_buf[(MAX_APP_MEMORY_SIZE / 4) - stack_size];
            // Calculate the heap size of the remaining memory
            uint32_t heap_size = ((MAX_APP_MEMORY_SIZE) - total_app_size) - (stack_size * 4);
            // Where is our heap going to start. It's directly after the ap + bss
            uint32_t *heap_entry = (uint32_t *)&app_stack_heap.byte_buf[total_app_size];

            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Base %x heap %x sz %d stack %x sz %d", 
                   app_stack_heap.word_buf,
                   heap_entry,
                   heap_size,
                   stack_entry, 
                   stack_size);
            
            // heap is all uint8_t
            appHeapInit(heap_size, (void *)heap_entry);

            // Let this guy do the heavy lifting!
            _app_task_handle = xTaskCreateStatic((TaskFunction_t)&app_stack_heap.byte_buf[header.offset], 
                                                 "dynapp", 
                                                 stack_size, 
                                                 NULL, 
                                                 tskIDLE_PRIORITY + 6UL, 
                                                 (StackType_t*) stack_entry, 
                                                 (StaticTask_t* )&_app_task);
        }
        else
        {
            // "System" or otherwise internal apps are spawned here. They don't need loading from flash,
            // just a reasonable entrypoint
            // The main loop work is deferred to the app until it quits
            appHeapInit(MAX_APP_MEMORY_SIZE - (MAX_APP_STACK_SIZE * 4), app_stack_heap.byte_buf);
             
            uint32_t *stack_entry = &app_stack_heap.word_buf[(MAX_APP_MEMORY_SIZE / 4) - MAX_APP_STACK_SIZE];
            
            DRV_LOG("app", APP_LOG_LEVEL_DEBUG, "Launching To the Moon!");
            
            _app_task_handle = xTaskCreateStatic((TaskFunction_t)_running_app->main, 
                                                  "dynapp", 
                                                  MAX_APP_STACK_SIZE, 
                                                  NULL, 
                                                  tskIDLE_PRIORITY + 6UL, 
                                                  stack_entry, 
                                                  &_app_task);

            // around we go again
            // TODO block while running
        }
    }
}

void back_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(_running_app->type)
    {
        case APP_TYPE_FACE:
            KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "TODO: Quiet time");
            break;
        case APP_TYPE_SYSTEM:
            // quit the app
            appmanager_app_start("Simple");
            break;
    }
}

void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context)
{
    
}

void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(_running_app->type)
    {
        case APP_TYPE_FACE:
            appmanager_app_start("System");
            break;
    }
}

void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    // Pop windows off
    Window *popped = window_stack_pop(true);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Window Count: %d", window_count());
    
    if (window_count() == 0)
    {
        appmanager_app_start("System");
    }
    window_dirty(true);
}

/*
 * Get the top level node for the app manifest
 */
App *app_manager_get_apps_head()
{
    return _app_manifest_head;
}

App *appmanager_get_running_app(void)
{
    return _running_app;
}



/* Some stubs below for testing etc */

void api_unimpl(void)
{

    while(1);
}

/* Some missing functionality */

void p_n_grect_standardize(n_GRect r)
{
    n_grect_standardize(r);
}



/*
 * Cheesy proxy to get the apps slot_id
 * When we need any resource from an app, we need a way of knowing
 * which app it was that wanted that resource. We know which app is running, that's the apps slot
 * 
 */
GBitmap *gbitmap_create_with_resource_proxy(uint32_t resource_id)
{
    return gbitmap_create_with_resource_app(resource_id, &_running_app->resource_file);
}

ResHandle resource_get_handle(uint16_t resource_id)
{
    return resource_get_handle_app(resource_id, &_running_app->resource_file);
}

void resource_load(ResHandle resource_handle, uint8_t *buffer, uint32_t size)
{
    // TODO: respect passed size, should we include file in ResHandle?
    return resource_load_app(resource_handle, buffer, &_running_app->resource_file);
}

// app proxies by pointer
ResHandle *resource_get_handle_proxy(uint16_t resource_id)
{
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "ResH %d %s", resource_id, _running_app->header->name);

    // push to the heap.
    ResHandle *x = app_malloc(sizeof(ResHandle));
    ResHandle y = resource_get_handle_app(resource_id, &_running_app->resource_file);
    memcpy(x, &y, sizeof(ResHandle));
     
    return x;
}

GFont *fonts_load_custom_font_proxy(ResHandle *handle)
{
    return (GFont *)fonts_load_custom_font(handle, &_running_app->resource_file);
}

