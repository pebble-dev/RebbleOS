/* appmanager.c
 * Routines for managing the loading of applications dynamically
 * Each app is loaded with its own stack and heap.
 *   (https://github.com/pebble-dev)
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebble_memory.h"
#include <stdlib.h>
#include "appmanager_thread.h"
#include "display.h"
#include "librebble.h"
#include "appmanager.h"

#include "systemapp.h"
#include "test.h"
#include "notification.h"
#include "api_func_symbols.h"
#include "qalloc.h"
#include "notification_manager.h"

/* Configure Logging */
#define MODULE_NAME "appman"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_ERROR

/*
 * Module TODO
 * 
 * Hook the flags up. These contain app type etc.
 * 
 */
// extern  GFont *fonts_load_custom_font(ResHandle*, struct file *file);

static void _app_management_thread(void *parameters);
static TaskHandle_t _app_thread_manager_task_handle;
static xQueueHandle _app_thread_queue;
static StaticTask_t _app_thread_manager_task;
static void _appmanager_thread_init(void *pvParameters);
static void _appmanager_update_progress_bar(uint32_t progress, uint32_t total, uint8_t type);
static void _running_app_loop(void);

enum {
    AppLoaded,
    AppInvalid,
};

/* The manager thread needs only a small stack */
#define APP_THREAD_MANAGER_STACK_SIZE 450
static StackType_t _app_thread_manager_stack[APP_THREAD_MANAGER_STACK_SIZE];  // stack + heap for app (in words)

/* keep these stacks off CCRAM */
static StackType_t _stack_app[MEMORY_SIZE_APP_STACK];
static StackType_t _stack_worker[MEMORY_SIZE_WORKER_STACK];
static StackType_t _stack_overlay[MEMORY_SIZE_OVERLAY_STACK];

/* Initialise everything we know about the different threads */
static app_running_thread _app_threads[MAX_APP_THREADS] = {
    {
        .thread_type = AppThreadMainApp,
        .thread_name = "MainApp",
        .heap = &mem_heaps[HEAP_APP],
        .stack_size = MEMORY_SIZE_APP_STACK,
        .stack = _stack_app,
        .thread_entry = &appmanager_app_main_entry,
        .thread_priority = 6UL,
    },
    {
        .thread_type = AppThreadWorker,
        .thread_name = "Worker",
        .heap = &mem_heaps[HEAP_WORKER],
        .stack_size = MEMORY_SIZE_WORKER_STACK,
        .stack = _stack_worker,
        /*.thread_entry = &appmanager_worker_main_entry */
        .thread_priority = 8UL,
    },
    {
        .thread_type = AppThreadOverlay,
        .thread_name = "Overlay",
        .heap = &mem_heaps[HEAP_OVERLAY],
        .stack_size = MEMORY_SIZE_OVERLAY_STACK,
        .stack = _stack_overlay,
        .thread_priority = 9UL,
    }
};

uint8_t appmanager_init(void)
{
    appmanager_app_loader_init();
    appmanager_app_runloop_init();

    _app_thread_queue = xQueueCreate(3, sizeof(struct AppMessage));

    appmanager_app_start("System");
    
    _app_thread_manager_task_handle = xTaskCreateStatic(_app_management_thread, 
                                                        "App", 
                                                        APP_THREAD_MANAGER_STACK_SIZE, 
                                                        NULL, 
                                                        tskIDLE_PRIORITY + 5UL, 
                                                        _app_thread_manager_stack, 
                                                        &_app_thread_manager_task);
    
    LOG_INFO("App thread created");
    
    return 0;
}

app_running_thread *appmanager_get_threads(void)
{
    return _app_threads;
}

app_running_thread *_get_current_thread(void)
{
    TaskHandle_t this_task = xTaskGetCurrentTaskHandle();
    
    for (uint8_t i = 0; i < MAX_APP_THREADS; i++)
    {
        if (_app_threads[i].task_handle == this_task)
            return &_app_threads[i];
    }
    
    return NULL;
}

/*
 * Get the currently executing thread context.
 * You will, in return, recieve a app_running_thread handle
 */
app_running_thread *appmanager_get_current_thread(void)
{
    app_running_thread *th = _get_current_thread();

//     assert(th && "I have no idea what thread I am running from! (System?)");
//     LOG_INFO("I have no idea what thread I am running from! (System?)");
    
    return th;
}


/*
 * Get the current thread's running ->app structure
 */
App *appmanager_get_current_app(void)
{
    app_running_thread *th = appmanager_get_current_thread();
    //assert(th->app && "There is no app!");
    
    return th->app;
}

/*
 * Send a message to the manager thread 
 */
bool appmanager_post_generic_thread_message(AppMessage *am, TickType_t timeout)
{
    return xQueueSendToBack(_app_thread_queue, am, (TickType_t)timeout);
}

app_running_thread *appmanager_get_thread(AppThreadType type)
{
    return &_app_threads[type];
}

AppThreadType appmanager_get_thread_type(void)
{
    app_running_thread *thread = appmanager_get_current_thread();
    return thread->thread_type;
}

/* 
 * Are we running as the system thread?
 */
bool appmanager_is_thread_system(void)
{
    app_running_thread *th = _get_current_thread();
    
    return (th == NULL);
}

bool appmanager_is_thread_overlay(void)
{
    return appmanager_get_thread_type() == AppThreadOverlay;
}

bool appmanager_is_thread_app(void)
{
    return appmanager_get_thread_type() == AppThreadMainApp;
}

bool appmanager_is_thread_worker(void)
{
    return appmanager_get_thread_type() == AppThreadWorker;
}

static inline bool _app_executes_from_internal_rom(App *app) {
    return app->flags & 1 << ExecuteFromInternalFlash;
}

static inline bool _app_file_present(App *app)
{
    return app->flags & 1 << AppFilePresent;
}

static inline bool _app_resource_file_present(App *app)
{
    return app->flags & 1 << ResourceFilePresent;
}

void appmanager_app_set_flag(App *app, uint8_t flag, bool value)
{
    app->flags = (app->flags & ~(1 << flag)) | ((value == true ? 1 : 0) << flag);
}

static void _draw(uint8_t state)
{
    static TickType_t _last_complete_draw = 0;
    static uint8_t _last_state = 2;

    if (!state && _last_state != 2 && _last_complete_draw + pdMS_TO_TICKS(250) < xTaskGetTickCount())
    {
        LOG_ERROR("We lost a draw packet! %d", _last_state);
        display_buffer_lock_give();
        _last_state = 2;
    }

    if (!state && !_last_state) {
        LOG_DEBUG("Draw in progress! %d", _last_state);
        return;
    }

    _last_state = state;
    
    switch (state)
    {
        case 0:
            /* Request a draw. This is mostly from an app invalidating something */
            if (display_buffer_lock_take(0))
            {
                _last_complete_draw = xTaskGetTickCount();
                if (appmanager_is_app_running())
                    appmanager_post_draw_app_message(1);
                else {
                    _draw(1);
                }
            }
            else
            {
                LOG_DEBUG("Lock Not Acquired");
                break;
            }
            break;
        case 1:            
            if (overlay_window_count() > 0) {
                overlay_window_draw(true);
            } else {
                _draw(2);
            }
            break;
        case 2:
            display_draw();
            display_buffer_lock_give();
            LOG_DEBUG("Render Time %dms", xTaskGetTickCount() - _last_complete_draw);
            break;
    }
}

static void _appmanager_thread_state_update(uint32_t app_id, uint8_t thread_id, AppMessage *am)
{
    app_running_thread *_this_thread = NULL;
    uint32_t total_app_size = 0;
    _this_thread = &_app_threads[thread_id];
    ApplicationHeader header;   /* TODO change to malloc so we can free after load? */
    
    if (_this_thread->status == AppThreadLoading || 
        _this_thread->status == AppThreadLoaded || 
        _this_thread->status == AppThreadRunloop)
    {
        return;
    }
    else if (_this_thread->status == AppThreadUnloading)
    {
        /* it's closing down */
        LOG_INFO("Waiting for app to close...");

        return;
    }
    else if (_this_thread->status == AppThreadDownloading)
    {
        if (_this_thread->app_start_tick > 0 && 
            _this_thread->app_start_tick + pdMS_TO_TICKS(6000) < xTaskGetTickCount()) {
                LOG_ERROR("Timed out loading app");
                _this_thread->status = AppThreadUnloaded;
                appmanager_app_start("System");
        }
    }
    else if (_this_thread->status == AppThreadUnloaded)
    {
        LOG_INFO("Starting app %d", app_id);
        _this_thread->status = AppThreadLoading;
        
        if (app_manager_get_apps_head() == NULL)
        {
            LOG_ERROR("No Apps found!");
            assert(!"No Apps");
            return;
        }

        App *app = appmanager_get_app_by_id(app_id);
        LOG_INFO("Starting app %s", app->name);
        
        if (app == NULL)
        {
            LOG_ERROR("App %d NOT found!", app_id);
            assert(!"App not found!");
            return;
        }

        /* We have an app that's at least known. push on with loading it */
        _this_thread->app = app;
        _this_thread->timer_head = NULL;
        
        /* At this point the existing task should be gone already
        * If it isn't we kill it. Lets complain though, becuase it's
        * broken if we are here */
        if (_this_thread->task_handle != NULL) {
            vTaskDelete(_this_thread->task_handle);
            _this_thread->task_handle = NULL;
            LOG_ERROR("The previous task was still running. FIXME");
        }
        
        mem_heap_init(_this_thread->heap);

        /* If the app is running off RAM (i.e it's a PIC loaded app...) 
        * and not system, we need to patch it */
        if (!_app_executes_from_internal_rom(app))
        {
            /* Check to see if we have app and resource files */
            if (!_app_file_present(app))
            {
                /* We now request the app from the host device */
                protocol_app_fetch_request(&app->uuid, app->id);
                _this_thread->status = AppThreadDownloading;
                
                notification_progress *prog = mem_heap_alloc(&mem_heaps[HEAP_LOWPRIO], sizeof(notification_progress));
                if (prog) {
                    memset(prog, 0, sizeof(*prog));
                    event_service_post(EventServiceCommandProgress, prog, (void *)remote_free);
                }
                
                LOG_INFO("Requesting App from host %x", app);
                return;
            }
        
            if (appmanager_load_app(_this_thread, &header) != AppLoaded)
                return;
            total_app_size = header.virtual_size;
        }
        else
        {
            total_app_size = 0;
        }
        
        /* Execute the app we just loaded */
        appmanager_execute_app(_this_thread, total_app_size);
    }
}

/*
 * A task to run an application.
 * 
 * Supervisory thread for all apps.
 * App launcher
 * Will kill a thread that seems dead
 * This task runs all the time and is a dynamic app loader and thread spawner
 * Once an app is loaded, it is handed off to a new task. 
 * The new task is created with a statically allocated array of it's memory size
 * This array is used as the heap and the stack.
 * refer to heap_app.c (for now, until the refactor) TODO
 * Once handoff is done, we sleep waitiing for a new job or something to kill
 * 
 * TODO
 * running thread ping pong for aliveness
 * Change out the kill timer for a local kill sleep
 */
static void _app_management_thread(void *parms)
{
    static char *app_name;
    static AppMessage am;
    static app_running_thread *_this_thread = NULL;
    static uint32_t _app_to_load_id = 1;
    static TickType_t _delay = portMAX_DELAY;

    for( ;; )
    {
        /* Sleep waiting for work to do */
        if (xQueueReceive(_app_thread_queue, &am, pdMS_TO_TICKS(_delay)))
        {
            _this_thread = &_app_threads[am.thread_id];
            switch(am.command)
            {
                /* Load an app for someone. face or worker */
                case THREAD_MANAGER_APP_LOAD_ID:
                    LOG_INFO("Quitting... %d", (uint32_t)am.data);
                    uint32_t id = (uint32_t)am.data;

                    _app_to_load_id = id;
                    if (_this_thread->status != AppThreadUnloaded) {
                        appmanager_app_quit_request();
                        _this_thread->app_start_tick = xTaskGetTickCount();
                    }
                    break;
                case THREAD_MANAGER_APP_DOWNLOAD_COMPLETE:
                    LOG_INFO("Download Complete %d", id);
                    App *capp = appmanager_get_app_by_id(_app_to_load_id);
                    assert(capp);
                    char buffer[14];

                    snprintf(buffer, 14, "@%08lx/app", capp->id);
                    if (fs_find_file(&capp->app_file, buffer) < 0) {
                        LOG_ERROR("App File %s not found", buffer);
                        appmanager_app_set_flag(capp, AppFilePresent, false);
                        _this_thread->status = AppThreadUnloaded;
                        break;
                    }
                    snprintf(buffer, 14, "@%08lx/res", capp->id);
                    if (fs_find_file(&capp->resource_file, buffer) < 0) {
                        LOG_ERROR("Res File %s not found", buffer);
                        appmanager_app_set_flag(capp, ResourceFilePresent, false);
                        appmanager_app_quit_done();
                        break;
                    }
                    appmanager_app_set_flag(capp, AppFilePresent, true);
                    appmanager_app_set_flag(capp, ResourceFilePresent, true);
                    
                    LOG_INFO("Download Complete %x", capp);

                     _this_thread->status = AppThreadUnloaded;
                    break;
                case THREAD_MANAGER_APP_QUIT_REQUEST:
                    if (_this_thread->status == AppThreadDownloading) {
                        break;
                    }
                    /* remove all of the clck handlers */
                    button_unsubscribe_all();

                    /* remove the ticktimer service handler and stop it */
                    tick_timer_service_unsubscribe_thread(_this_thread);
                    connection_service_unsubscribe_thread(_this_thread);
                    event_service_unsubscribe_thread_all(_this_thread);
                    
                    appmanager_app_quit();

                    /* Set the shutdown time for this app. We will kill it then */
                    if (!appmanager_is_app_shutting_down()) {
                        _delay = _this_thread->shutdown_at_tick = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
                        _this_thread->status = AppThreadUnloading;
                    }
                    break;
                case THREAD_MANAGER_APP_HEARTBEAT:
                    /* App is pulsing to us. lets update its kill time */
                    _this_thread->shutdown_at_tick = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
                    break;
                case THREAD_MANAGER_APP_TEARDOWN:
                    if (_this_thread->status != AppThreadUnloading)
                        LOG_WARN("Unloading app while not in correct state!");
                    
                    /* We were signalled that the app has finished cleanly */
                    LOG_DEBUG("Tearing app down %x", am.thread_id);
                    
                    /* The task will die hard now */
                    assert(_this_thread->task_handle);

                    vTaskDelay(2); /* We yield to the thread to it can sit in wait */
                    vTaskDelete(_this_thread->task_handle);
                    _this_thread->task_handle = NULL;
                    _this_thread->shutdown_at_tick = 0;
                    _this_thread->app = NULL;
                    _this_thread->status = AppThreadUnloaded;

                    _delay = portMAX_DELAY;
                    break;
                case THREAD_MANAGER_APP_DRAW:
                    _draw((uint32_t)am.data);
                    break;
            }
        }
    
        _appmanager_thread_state_update(_app_to_load_id, am.thread_id, &am);
        /* We woke up to check if we need to kill anything */
        /* check all threads */
        for (uint8_t i = 0; i < MAX_APP_THREADS; i++)
        {
            _this_thread = &_app_threads[i];
            if (_this_thread->shutdown_at_tick > 0 &&
                    xTaskGetTickCount() >= _this_thread->shutdown_at_tick)
            {
                LOG_ERROR("!! Hard terminating app");
                
                vTaskDelete(_this_thread->task_handle);
                _this_thread->shutdown_at_tick = 0;
                _this_thread->status = AppThreadUnloaded;
            }
            /* app really should have died by now */
        }

        /* around we go again */
    }
}


/*
    Heres what is going down. We are going to load the app from flash.
    The app is stored in flash and is a butchered ELF position independant
    compiled (-fPIC) library. It's layout is as so:
    
     | AppicationHeader | App Binary (.text) | Data Section (.data) | Global offset  | BSS [and reloc table] |
        
    There is a symbol table in each Pebble app that needs to have the address 
    of _our_ symbol table poked into it. 
    The symbol table is a big lookup table of pointer functions. 
    When an app calls a function such as window_create() it actually turns 
    that into an integer id in the app.
    When the app calls the function, it does 
      function => id => RebbleOS => id to function => call
    This allows the app to do a lookup of a function id internally, check 
    it in our rebble sym table, and return a pointer address to to the function 
    to jump to.
    
    We also have to take care of BSS section. This is virtual size - app size
    BSS. is always zeroed.
    
    Then relocate entries. Each Pebble app is compiled with 
    Position Independant code (PIC) that means it can run from any address.
    This is usually used on dlls .so files and the like, and a Pebble app 
    is no different. It is the responsibility of the dynamic loader 
    (us in this case) to get the list of relocations (which is at the end of 
    the binary) and "patch" or relocate all of the data symbols in the 
    application binary.
    
    (see)
    http://grantcurell.com/2015/09/21/what-is-the-symbol-table-and-what-is-the-global-offset-table/
    
    When the app executes, any variables stored in the global are looked up...
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
    from, while the stack is uint32.
    Each app is given its own stack, while the heap is shared between the app
    binary and the app memory
    
    We are leaning on Rtos here to actually spawn the app with it's own stack
    (cheap fork)  and make sure the (M)SP is set accordingly. 
    As a bonus we can manipulate this task in FreeRTOS directly through it's 
    TCB handle (suspen, kill, delete)
    
    Steps:
    * Find app on flash
    * Load app into lower stack
    * Set symbol table address
    * load relocs and reloc the GOT + DATA
    * zero BSS
    * fork
        
    */
int appmanager_load_app(app_running_thread *thread, ApplicationHeader *header)
{   
    struct fd fd;
    
    fs_open(&fd, &thread->app->app_file);
    fs_read(&fd, header, sizeof(ApplicationHeader));

    /* sanity check the hell out of this to make sure it's a real app */
    if (strncmp(header->header, "PBLAPP", 6))
    {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "No PBLAPP header!");
        return AppInvalid;
    }
    
    void *app = mem_heap_alloc(thread->heap, header->app_size + (header->reloc_entries_count * 4));
    if (!app) {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "no RAM for app (sz %d)", header->app_size + (header->reloc_entries_count * 4));
        return AppInvalid;
    }
    
    /* load the app from flash
     *  and any reloc entries too. */
    fs_seek(&fd, 0, FS_SEEK_SET);
    fs_read(&fd, app, header->app_size + (header->reloc_entries_count * 4));
    
    /* apps get loaded into heap like so
     * [App Header | App Binary | App Heap | App Stack]
     */
       
    /* re-allocate the GOT for -fPIC
     * A normal ELF dyn loader would look at the ELF header and relocate
     * any addresses that need to be relocated. On a pebble, we only have the
     * bin plus a table of relocations appeneded to the end.
     * This table has the offset from app bin start to the register to reloc
     * Some of the reloc will be from the .DATA section, some will be .GOT
     * (global offset table) entries
     * The reloc table actually sits in BSS section, so once we have done
     * we wipe the BSS again. */
    uint8_t *reloc_table_addr = app + header->app_size;
    
    /* Now we have the relocs to do, we are in standard ELF dyn loader mode 
     * (albeit without having to deal with relocating PLTs)
     * Each register to be relocated will be a relative offset.
     * To make it all work we:
     * address with relative offset = address of app bin + relative offset
     */    
    if (header->reloc_entries_count > 0)
    {                
        /* go through all of the reloc entries and do the reloc dance */
        for (uint32_t i = 0; i < header->reloc_entries_count; i++)
        {
            uint32_t reloc_idx = i * 4;
            /* get the offset from app base to the register to relocate */
            uint32_t reg_to_reloc = read_32(&reloc_table_addr[reloc_idx]);          
            assert(reg_to_reloc < header->virtual_size && "Reloc entry beyond app bounds");
            
            /* Get the value from the register we are relocating.
             * This will contain the offset from the app base to the data */
            uint32_t rel_off = read_32(app + reg_to_reloc);
            
            /* Add the app base absolute memory register address to the offset
             * Write this absolute value back into the register to relocate */
            write_32(app + reg_to_reloc, (uint32_t)((uintptr_t)(app + rel_off)));
        }
    }
    
    /* init bss to 0. We already zeros all of the heap, so only reset the reloc table */
    uint32_t bss_size = header->virtual_size - header->app_size;
    
    app = mem_heap_realloc(thread->heap, app, header->virtual_size);
    if (!app) {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "no RAM for app BSS (size %d)", header->virtual_size);
        return AppInvalid;
    }
    memset(app + header->app_size, 0, bss_size);
    memset(thread->stack, 0, thread->stack_size * 4);
    
    /* load the address of our lookup table into the 
     * special register in the app. */
    write_32(app + header->sym_table_addr, (int32_t)sym);
     
    /* Patch the app's entry point... make sure its THUMB bit set! */
    thread->app->main = (AppMainHandler)((uint32_t)(app + header->offset) | 1);
    
    LOG_DEBUG("== App signature ==");
    LOG_DEBUG("Header  : %s",    header->header);
    LOG_DEBUG("SDK ver : %d.%d", header->sdk_version.major, 
                                                            header->sdk_version.minor);
    LOG_DEBUG("App ver : %d.%d", header->app_version.major, 
                                                            header->app_version.minor);
    LOG_DEBUG("App Size: 0x%x",  header->app_size);
    LOG_DEBUG("App Main: 0x%x",  header->offset);
    LOG_DEBUG("CRC     : 0x%x",  header->crc);
    LOG_DEBUG("Name    : %s",    header->name);
    LOG_DEBUG("Company : %s",    header->company);
    LOG_DEBUG("Icon    : %d",    header->icon_resource_id);
    LOG_DEBUG("Pbl Sym : 0x%x",  header->sym_table_addr);
    LOG_DEBUG("Flags   : %d",    header->flags);
    LOG_DEBUG("Reloc   : %d",    header->reloc_entries_count);
    LOG_DEBUG("== Memory signature ==");
    LOG_DEBUG("VSize   : 0x%x",  header->virtual_size);
    LOG_DEBUG("Bss Size: %d",    bss_size);
    LOG_DEBUG("Heap    : 0x%x",  app + header->virtual_size);
    
    return AppLoaded;
}

/* 
 * Given a thread, execute the application inside of a vTask
 */
void appmanager_execute_app(app_running_thread *thread, uint32_t total_app_size)
{    

    LOG_DEBUG("exec app: stack 0x%x, sz %d (w)", 
            thread->stack, 
            thread->stack_size);
    
    /* Load the app in a vTask */
    xTaskCreateStatic(_appmanager_thread_init, 
                        thread->thread_name, 
                        thread->stack_size, 
                        (void *)thread, 
                        tskIDLE_PRIORITY + thread->thread_priority, 
                        thread->stack, 
                        (StaticTask_t*)&thread->static_task);
}

static void _appmanager_thread_init(void *thread_handle)
{
    app_running_thread *thread = (app_running_thread *)thread_handle;
    assert(thread && "Invalid thread on init!");
    thread->task_handle = xTaskGetCurrentTaskHandle();
    ((VoidFunc)thread->thread_entry)();
}

