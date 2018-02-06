/* appmanager_app_runloop.c
 * The main runloop and runloop handlers for the main foregound App
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"

void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context);

static xQueueHandle _app_message_queue;

void appmanager_app_runloop_init(void)
{
    _app_message_queue = xQueueCreate(5, sizeof(struct AppMessage));    
}

/* 
 * Send a message to an app 
 */
void appmanager_post_generic_app_message(AppMessage *am, TickType_t timeout)
{
    xQueueSendToBack(_app_message_queue, am, timeout);
}

/*
 * We are the main entrypoint for running a thread.
 * When we are done, we notify the main thread we shutdown
 * lest we get murdered
 */
void appmanager_app_main_entry(void)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    
    _this_thread->status = AppThreadLoaded;
    /* Call into the apps main runtime */
    _this_thread->app->main();
    _this_thread->status = AppThreadUnloading;
    
    AppMessage am = {
        .thread_id = _this_thread->thread_type,
        .message_type_id = THREAD_MANAGER_APP_QUIT_CLEAN,
    };
    
    appmanager_post_generic_thread_message(&am, 100);
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "App Finished.");
    
    /* We are done with our app. Block until we are killed */
    vTaskDelay(portMAX_DELAY);
}

/*
 * Once an application is spawned, it calls into app_event_loop
 * This function is a busy loop, but with the benefit that it is also a task
 * In here we are the main event handler, for buttons quits etc etc.
 */
void app_event_loop(void)
{
    AppMessage data;
    app_running_thread *_this_thread = appmanager_get_current_thread();
    App *_running_app = _this_thread->app;
    
    if (_this_thread->thread_type != AppThreadMainApp)
    {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Naughty! You tried to run an app runloop!. You are not an app");
        return;
    }
    fonts_resetcache();
    
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App entered mainloop");
    
    /* Do this before window load, that way they have a chance to override */
    if (_running_app->type != APP_TYPE_FACE)
    {
        /* Enables default closing of windows, and through that, apps */
        window_single_click_subscribe(BUTTON_ID_BACK, app_back_single_click_handler);
    }
    
    window_configure(window_stack_get_top_window());
    
    /* Install our own handler to hijack the long back press
     * window_long_click_subscribe(BUTTON_ID_BACK, 1100, back_long_click_handler, back_long_click_release_handler);
     */
    
    if (_running_app->type != APP_TYPE_SYSTEM)
    {
        window_single_click_subscribe(BUTTON_ID_SELECT, app_select_single_click_handler);
    }
    
    
    /* clear the queue of any work from the previous app
    * ... such as an errant quit */
    xQueueReset(_app_message_queue);

    window_draw();
    
    /* App is now fully initialised and inside the runloop. */
    for ( ;; )
    {
        /* Is there something queued up to do?  If so, we have the potential to do it. */
        TickType_t next_timer;
        
        if (_this_thread->timer_head) {
            TickType_t curtime = xTaskGetTickCount();
            if (curtime > _this_thread->timer_head->when)
                next_timer = 0;
            else
                next_timer = _this_thread->timer_head->when - curtime;
        } else {
            next_timer = portMAX_DELAY; /* Just block forever. */
        }
        
        /* we are inside the apps main loop event handler now */
        if (xQueueReceive(_app_message_queue, &data, next_timer))
        {
            /* We woke up for some kind of event that someone posted.  But what? */
            KERN_LOG("app", APP_LOG_LEVEL_INFO, "Queue Receive");
            if (data.message_type_id == APP_BUTTON)
            {
                /* execute the button's callback */
                ButtonMessage *message = (ButtonMessage *)data.payload;
                ((ClickHandler)(message->callback))((ClickRecognizerRef)(message->clickref), message->context);
            }
            else if (data.message_type_id == APP_QUIT)
            {
                /* remove all of the clck handlers */
                button_unsubscribe_all();
                /* remove the ticktimer service handler and stop it */
                tick_timer_service_unsubscribe();

                /* Set the shutdown time for this app. We will kill it then */
                _this_thread->shutdown_at_tick = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
                
                KERN_LOG("app", APP_LOG_LEVEL_INFO, "App Quit");

                /* app was quit, break out of this loop into the main handler */
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
            CoreTimer *timer = _this_thread->timer_head;
            assert(timer);
            
            KERN_LOG("app", APP_LOG_LEVEL_INFO, "woke up for a timer");

            _this_thread->timer_head = timer->next;
            
            timer->callback(timer);
        }
    }
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App Signalled shutdown...");
    /* We fall out of the apps main_ now and into deinit and thread completion
     * We will hand back control to appmanager_app_main_entry above */
}


/* Apps click handlers */

void back_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    switch(_this_thread->app->type)
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
    app_running_thread *_this_thread = appmanager_get_current_thread();
    switch(_this_thread->app->type)
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
