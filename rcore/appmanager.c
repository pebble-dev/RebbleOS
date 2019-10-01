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
#include "librebble.h"
#include "appmanager.h"
#include "systemapp.h"
#include "test.h"
#include "notification.h"
#include "api_func_symbols.h"
#include "qalloc.h"

/* Configure Logging */
#define MODULE_NAME "appman"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

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

static void _running_app_loop(void);

/* The manager thread needs only a small stack */
#define APP_THREAD_MANAGER_STACK_SIZE 450
static StackType_t _app_thread_manager_stack[APP_THREAD_MANAGER_STACK_SIZE];  // stack + heap for app (in words)

/* Our pre allocated heaps for the different threads 
 * it's tempting to over engineer this and make it a node list
 * or at least add dynamicness to it. But honestly we have 3 threads
 * max at the moment, so if we get there, maybe */
static uint8_t _heap_app[MEMORY_SIZE_APP_HEAP];
static MEM_REGION_HEAP_WRK uint8_t _heap_worker[MEMORY_SIZE_WORKER_HEAP];
static MEM_REGION_HEAP_OVL uint8_t _heap_overlay[MEMORY_SIZE_OVERLAY_HEAP];

/* keep these stacks off CCRAM */
static StackType_t _stack_app[MEMORY_SIZE_APP_STACK];
static StackType_t _stack_worker[MEMORY_SIZE_WORKER_STACK];
static StackType_t _stack_overlay[MEMORY_SIZE_OVERLAY_STACK];

/* Initialise everything we know about the different threads */
static app_running_thread _app_threads[MAX_APP_THREADS] = {
    {
        .thread_type = AppThreadMainApp,
        .thread_name = "MainApp",
        .heap_size = MEMORY_SIZE_APP_HEAP,
        .heap = _heap_app,
        .stack_size = MEMORY_SIZE_APP_STACK,
        .stack = _stack_app,
        .thread_entry = &appmanager_app_main_entry,
        .thread_priority = 6UL,
    },
    {
        .thread_type = AppThreadWorker,
        .thread_name = "Worker",
        .heap_size = MEMORY_SIZE_WORKER_HEAP,
        .heap = _heap_worker,
        .stack_size = MEMORY_SIZE_WORKER_STACK,
        .stack = _stack_worker,
        /*.thread_entry = &appmanager_worker_main_entry */
        .thread_priority = 8UL,
    },
    {
        .thread_type = AppThreadOverlay,
        .thread_name = "Overlay",
        .heap_size = MEMORY_SIZE_OVERLAY_HEAP,
        .heap = _heap_overlay,
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
    assert(th->app && "There is no app!");
    
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
    ApplicationHeader header;   /* TODO change to malloc so we can free after load? */
    char *app_name;
    AppMessage am;
    uint32_t total_app_size = 0;
    app_running_thread *_this_thread = NULL;
    
    for( ;; )
    {
        /* Sleep waiting for work to do */
        if (xQueueReceive(_app_thread_queue, &am, pdMS_TO_TICKS(1000)))
        {
            switch(am.command)
            {
                /* Load an app for someone. face or worker */
                case THREAD_MANAGER_APP_LOAD:
                    app_name = (char *)am.data;
                    _this_thread = &_app_threads[am.thread_id];

                    if (_this_thread->status == AppThreadLoading || 
                        _this_thread->status == AppThreadLoaded || 
                        _this_thread->status == AppThreadRunloop)
                    {
                        /* post an app quit to the running process
                         * then immediately post our received message back onto 
                         * the queue behind the quit. 
                         * Once the app is quit, we will continue 
                         * TODO we could go around this merry-go-round for a while
                         * we should track that or use a better mechanism.
                         * in reality this isn't a big issue. A concern maybe
                         */
                        LOG_INFO("Quitting...");
                        appmanager_app_quit();
                        xQueueSendToBack(_app_thread_queue, &am, (TickType_t)100);
                        continue;
                    }
                    else if (_this_thread->status == AppThreadUnloading)
                    {
                        /* it's closing down, keep reposting the message 
                         * until we are free to launch 
                         * TODO: this is pretty weak 
                         * Make this a proper state machine, wait on completion 
                         * and launch the app
                         */
                        LOG_INFO("Waiting for app to close...");
                        vTaskDelay(pdMS_TO_TICKS(10));
                        xQueueSendToBack(_app_thread_queue, &am, (TickType_t)100);
                        continue;
                    }
                    
                    LOG_INFO("Starting app %s", app_name);
                    _this_thread->status = AppThreadLoading;
                        
                    /*  TODO reset clicks */
                    tick_timer_service_unsubscribe();
                    
                    if (app_manager_get_apps_head() == NULL)
                    {
                        LOG_ERROR("No Apps found!");
                        assert(!"No Apps");
                        return;
                    }
                    
                    App *app = appmanager_get_app(app_name);
                    
                    if (app == NULL)
                    {
                        LOG_ERROR("App %s NOT found!", app_name);
                        assert(!"App not found!");
                        continue;
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
                    
                    /* If the app is running off RAM (i.e it's a PIC loaded app...) 
                    * and not system, we need to patch it */
                    if (!app->is_internal)
                    {
                        appmanager_load_app(_this_thread, &header);
                        total_app_size = header.virtual_size;
                    }
                    else
                    {
                        total_app_size = 0;
                    }
                    
                    /* Execute the app we just loaded */
                    appmanager_execute_app(_this_thread, total_app_size);
                    break;
                case THREAD_MANAGER_APP_QUIT_CLEAN:
                    _this_thread = &_app_threads[am.thread_id];
                    
                    if (_this_thread->status != AppThreadUnloading)
                        LOG_WARN("Unloading app while not in correct state!");
                    
                    /* We were signalled that the app has finished cleanly */
                    LOG_DEBUG("App finished cleanly");
                    
                    /* The task will die hard, but it did finish the runloop */
                    vTaskDelete(_this_thread->task_handle);
                    _this_thread->task_handle = NULL;
                    _this_thread->shutdown_at_tick = 0;
                    _this_thread->app = NULL;
                    _this_thread->status = AppThreadUnloaded;
                    break;
            }        
        }
        else
        {
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
void appmanager_load_app(app_running_thread *thread, ApplicationHeader *header)
{   
    struct fd fd;
    
    /* de-fluff */
    memset(thread->heap, 0, thread->heap_size);

    fs_open(&fd, &thread->app->app_file);
    fs_read(&fd, header, sizeof(ApplicationHeader));

    /* load the app from flash
     *  and any reloc entries too. */
    fs_seek(&fd, 0, FS_SEEK_SET);
    fs_read(&fd, thread->heap, header->app_size + (header->reloc_entries_count * 4));
    
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
    uint8_t *reloc_table_addr = thread->heap + header->app_size;
    
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
            uint32_t rel_off = read_32(thread->heap + reg_to_reloc);
            
            /* Add the app base absolute memory register address to the offset
             * Write this absolute value back into the register to relocate */
            write_32(thread->heap + reg_to_reloc, (uint32_t)((uintptr_t)(thread->heap + rel_off)));
        }
    }
    
    /* init bss to 0. We already zeros all of the heap, so only reset the reloc table */
    uint32_t bss_size = header->virtual_size - header->app_size;
    
    memset(thread->heap + header->app_size, 0, header->reloc_entries_count * 4);
    memset(thread->stack, 0, thread->stack_size * 4);
    
    /* load the address of our lookup table into the 
     * special register in the app. */
    write_32(&thread->heap[header->sym_table_addr], (int32_t)sym);

    assert(read_32(&thread->heap[header->sym_table_addr]) == (int32_t)sym && "PLT rewrite failed");
     
    /* Patch the app's entry point... make sure its THUMB bit set! */
    thread->app->main = (AppMainHandler)((uint32_t)&thread->heap[header->offset] | 1);
    
    LOG_DEBUG("== App signature ==");
    LOG_DEBUG("Header  : %s",    header->header);
    LOG_DEBUG("SDK ver : %d.%d", header->sdk_version.major, 
                                                            header->sdk_version.minor);
    LOG_DEBUG("App ver : %d.%d", header->app_version.major, 
                                                            header->app_version.minor);
    LOG_DEBUG("App Size: 0x%x",  header->app_size);
    LOG_DEBUG("App Main: 0x%x",  header->offset);
    LOG_DEBUG("CRC     : %d",    header->crc);
    LOG_DEBUG("Name    : %s",    header->name);
    LOG_DEBUG("Company : %s",    header->company);
    LOG_DEBUG("Icon    : %d",    header->icon_resource_id);
    LOG_DEBUG("Pbl Sym : 0x%x",  header->sym_table_addr);
    LOG_DEBUG("Flags   : %d",    header->flags);
    LOG_DEBUG("Reloc   : %d",    header->reloc_entries_count);
    LOG_DEBUG("== Memory signature ==");
    LOG_DEBUG("VSize   : 0x%x",  header->virtual_size);
    LOG_DEBUG("Bss Size: %d",    bss_size);
    LOG_DEBUG("Heap    : 0x%x",  thread->heap + header->virtual_size);
}

/* 
 * Given a thread, execute the application inside of a vTask
 */
void appmanager_execute_app(app_running_thread *thread, uint32_t total_app_size)
{    
    /* Calculate the heap size of the remaining memory */
    uint32_t heap_size = thread->heap_size - total_app_size;
    /* Where is our heap going to start. It's directly after the app + bss */
    uint8_t *heap_entry = &thread->heap[total_app_size];

    LOG_DEBUG("Exec: Base 0x%x, heap 0x%x, sz %d (b), stack 0x%x, sz %d (w)", 
            thread->heap,
            heap_entry,
            heap_size,
            thread->stack, 
            thread->stack_size);
    
    /* heap is all uint8_t */
    thread->arena = qinit(heap_entry, heap_size);
    
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

