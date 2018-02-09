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
#include "qalloc.h"

/*
 * Module TODO
 * 
 * Hook the flags up. These contain app type etc.
 * 
 */
extern  GFont *fonts_load_custom_font(ResHandle*, struct file *file);

static void _app_management_thread(void *parameters);
static TaskHandle_t _app_thread_manager_task_handle;
static xQueueHandle _app_thread_queue;
static StaticTask_t _app_thread_manager_task;
static StaticTask_t _app_task;

static void _running_app_loop(void);

/* The manager thread needs only a small stack */
#define APP_THREAD_MANAGER_STACK_SIZE 300
StackType_t _app_thread_manager_stack[APP_THREAD_MANAGER_STACK_SIZE];  // stack + heap for app (in words)


/* Our pre allocated heaps for the different threads 
 * it's tempting to over engineer this and make it a node list
 * or at least add dynamicness to it. But honestly we have 3 threads
 * max at the moment, so if we get there, maybe */
uint8_t _heap_app[MEMORY_SIZE_APP];
CCRAM uint8_t _heap_worker[MEMORY_SIZE_WORKER];
CCRAM uint8_t _heap_overlay[MEMORY_SIZE_OVERLAY];

/* Initialise everything we know about the different threads */
app_running_thread _app_threads[MAX_APP_THREADS] = {
    {
        .thread_type = AppThreadMainApp,
        .thread_name = "MainApp",
        .heap_size = MEMORY_SIZE_APP,
        .heap = (app_stack_heap_proto*)&_heap_app,
        .stack_size = MEMORY_SIZE_APP_STACK,
        .thread_entry = &appmanager_app_main_entry,
        .thread_priority = 6UL,
    },
    {
        .thread_type = AppThreadWorker,
        .thread_name = "Worker",
        .heap_size = MEMORY_SIZE_WORKER,
        .heap = (app_stack_heap_proto*)&_heap_worker,
        .stack_size = MEMORY_SIZE_WORKER_STACK,
        /*.thread_entry = &appmanager_worker_main_entry */
        .thread_priority = 8UL,
    },
    {
        .thread_type = AppThreadOverlay,
        .thread_name = "Overlay",
        .heap_size = MEMORY_SIZE_OVERLAY,
        .heap = (app_stack_heap_proto*)&_heap_overlay,
        .stack_size = MEMORY_SIZE_OVERLAY_STACK,
        .thread_priority = 9UL,
    }
};


void appmanager_init(void)
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
    
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App thread created");
}



/* 
 * Always adds to the running app's queue.  Note that this is only
 * reasonable to do from the app thread: otherwise, you can race with the
 * check for the timer head.
 */
void appmanager_timer_add(CoreTimer *timer)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    CoreTimer **tnext = &_this_thread->timer_head;
    
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
    app_running_thread *_this_thread = appmanager_get_current_thread();
    CoreTimer **tnext = &_this_thread->timer_head;
    
    while (*tnext) {
        if (*tnext == timer) {
            *tnext = timer->next;
            return;
        }
        tnext = &(*tnext)->next;
    }
    
    assert(!"appmanager_timer_remove did not find timer in list");
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
    assert(th && "I have no idea what thread I am running from! (System?)");
    
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
 * Are we running as the system thread?
 */
bool appmanager_is_thread_system(void)
{
    app_running_thread *th = _get_current_thread();
    
    return (th == NULL);
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
            switch(am.message_type_id)
            {
                /* Load an app for someone. face or worker */
                case THREAD_MANAGER_APP_LOAD:
                    app_name = (char *)am.payload;
                    _this_thread = &_app_threads[am.thread_id];
                    
                    if (_this_thread->status == AppThreadLoading || 
                        _this_thread->status == AppThreadLoaded)
                    {
                        /* post an app quit to the running process
                         * then immediately post our received message back onto 
                         * the queue behind the quit. 
                         * Once the app is quit, we will continue 
                         * TODO we could go around this merry-go-round for a while
                         * we should track that or use a better mechanism.
                         * in reality this isn't a big issue. A concern maybe
                         */
                        appmanager_app_quit();
                        xQueueSendToBack(_app_thread_queue, &am, (TickType_t)100);
                        continue;
                    }
                    
                    KERN_LOG("app", APP_LOG_LEVEL_INFO, "Starting app %s", app_name);
                    _this_thread->status = AppThreadLoading;
                        
                    /*  TODO reset clicks */
                    tick_timer_service_unsubscribe();
                    
                    if (app_manager_get_apps_head() == NULL)
                    {
                        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "No Apps found!");
                        assert(!"No Apps");
                        return;
                    }
                    
                    App *app = appmanager_get_app(app_name);
                    
                    if (app == NULL)
                    {
                        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "App %s NOT found!", app_name);
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
                        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "The previous task was still running. FIXME");
                    }
                    
                    /* If the app is running off RAM (i.e it's a PIC loaded app...) 
                    * and not system, we need to patch it */
                    if (!app->is_internal)
                    {
                        appmanager_load_app(_this_thread, &header);
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
                        KERN_LOG("app", APP_LOG_LEVEL_WARNING, "Unloading app while not in correct state!");
                    
                    /* We were signalled that the app has finished cleanly */
                    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "App finished cleanly");
                    
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
                    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "!! Hard terminating app");
                    
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
void appmanager_load_app(app_running_thread *thread, ApplicationHeader *header)
{
    app_stack_heap_proto *app_stack_heap = (app_stack_heap_proto *)thread->heap;
    
    struct fd fd;
    fs_open(&fd, &thread->app->app_file);
    fs_read(&fd, header, sizeof(ApplicationHeader));

    /* load the app from flash
     *  and any reloc entries too. */
    fs_seek(&fd, 0, FS_SEEK_SET);
    fs_read(&fd, app_stack_heap->byte_buf, header->app_size + (header->reloc_entries_count * 4));
    
    /* apps get loaded into heap like so
     * [App Header | App Binary | App Heap | App Stack]
     */
    
    /* re-allocate the GOT for -fPIC
     * Pebble has the GOT at the end of the app.
     * first get the GOT realloc table */
    uint32_t *got = &app_stack_heap->word_buf[header->app_size / 4];

    if (header->reloc_entries_count > 0)
    {                
        /* go through all of the reloc entries and do the reloc dance */
        for (uint32_t i = 0; i < header->reloc_entries_count; i++)
        {
            /* get the got out of the table. */
            /* use that value to get the relative offset from the address */
            uint32_t existing = app_stack_heap->word_buf[got[i]/4];

            /* we are working in words */
            existing /= 4;
            
            /* take the offset and add the apps base address
             * We are doing some nasty things with pointers here, where we are getting the base address
             * adding the offset in the register and forcing its new adderss back in
             * here we are going to go through a uint pointer for type safety */
            uintptr_t wb = (uintptr_t)(app_stack_heap->word_buf + existing);
            
            /* write it back to the register */
            app_stack_heap->word_buf[got[i]/4] = wb;
        }
    }
    
    /* init bss to 0 */
    uint32_t bss_size = header->virtual_size - header->app_size;
    memset(&app_stack_heap->byte_buf[header->app_size], 0, bss_size);
    
    /* load the address of our lookup table into the 
     * special register in the app. hopefully in a platformish independant way */
    app_stack_heap->byte_buf[header->sym_table_addr]     =  (uint32_t)(sym)        & 0xFF;
    app_stack_heap->byte_buf[header->sym_table_addr + 1] = ((uint32_t)(sym) >> 8)  & 0xFF;
    app_stack_heap->byte_buf[header->sym_table_addr + 2] = ((uint32_t)(sym) >> 16) & 0xFF;
    app_stack_heap->byte_buf[header->sym_table_addr + 3] = ((uint32_t)(sym) >> 24) & 0xFF;
    
    /* Patch the app's entry point... make sure it's THUMB bit set! */
    thread->app->main = (AppMainHandler)((uint32_t)&app_stack_heap->byte_buf[header->offset] | 1);
    
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "App signature:");
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "H:    %s",    header->header);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "SDKv: %d.%d", header->sdk_version.major, 
                                                        header->sdk_version.minor);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Appv: %d.%d", header->app_version.major, 
                                                        header->app_version.minor);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppSz:%x",    header->app_size);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppOf:0x%x",  header->offset);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "AppCr:%d",    header->crc);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Name: %s",    header->name);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Cmpy: %s",    header->company);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Icon: %d",    header->icon_resource_id);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Sym:  0x%x",  header->sym_table_addr);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Flags:%d",    header->flags);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Reloc:%d",    header->reloc_entries_count);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "VSize 0x%x",  header->virtual_size);
}

/* 
 * Given a thread, execute the application inside of a vTask
 */
void appmanager_execute_app(app_running_thread *thread, int total_app_size)
{    
    uint32_t stack_size = thread->stack_size;
    app_stack_heap_proto *app_stack_heap = (app_stack_heap_proto *)thread->heap;
    
    /* Get the start point of the stack in the array */
    uint32_t *stack_entry = &app_stack_heap->word_buf[(thread->heap_size - total_app_size - stack_size) / 4];
    /* Calculate the heap size of the remaining memory */
    uint32_t heap_size = thread->heap_size - total_app_size - stack_size;
    /* Where is our heap going to start. It's directly after the app + bss */
    uint32_t *heap_entry = (uint32_t *)&app_stack_heap->byte_buf[total_app_size];

    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "Exec: Base %x, heap %x, sz %d (b), stack %x, sz %d (w)", 
            app_stack_heap->word_buf,
            heap_entry,
            heap_size,
            stack_entry, 
            stack_size / 4);
    
    /* heap is all uint8_t */
    thread->arena = qinit(heap_entry, heap_size);
    
    /* Load the app in a vTask */
    thread->task_handle = xTaskCreateStatic((TaskFunction_t)thread->thread_entry, 
                                            "dynapp", 
                                            stack_size / 4, 
                                            NULL, 
                                            tskIDLE_PRIORITY + thread->thread_priority, 
                                            (StackType_t*) stack_entry, 
                                            (StaticTask_t* )&_app_task);
}

/*
 * Send a message to the manager thread 
 */
bool appmanager_post_generic_thread_message(AppMessage *am, TickType_t timeout)
{
    return xQueueSendToBack(_app_thread_queue, am, (TickType_t)timeout);
}

/* Some stubs below for testing etc */
void api_unimpl(void)
{
    while(1);
}
