/* appmanager_app_runloop.c
 * The main runloop and runloop handlers for the main foregound App
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "overlay_manager.h"
#include "notification_manager.h"

void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void app_back_single_click_handler(ClickRecognizerRef recognizer, void *context);
bool booted = false;

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
    
    /* Before we even see them, we have to reset fonts -- otherwise, the
     * font cache does some free business on old pointers, corrupting the
     * heap before we even had a fighting chance!  */
    fonts_resetcache();
    
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

static bool _app_shutting_down(void)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    return _this_thread->status == AppThreadUnloading;
}

void rocky_event_loop_with_resource(uint16_t resource_id)
{
    app_event_loop();
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
    bool draw_requested = false;
    
    if (_this_thread->thread_type != AppThreadMainApp)
    {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Runloop: You are not an app");
        return;
    }
    
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App entered mainloop");
    
    /* Do this before window load, that way they have a chance to override */
    if (_running_app->type != APP_TYPE_FACE &&
        overlay_window_count() == 0)
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

    if (!booted)
    {
        GRect frame = GRect(0, DISPLAY_ROWS - 20, DISPLAY_COLS, 20);
        notification_show_small_message("Welcome to RebbleOS", frame);
        booted = true;
    }
    
    TickType_t next_timer;
    
    /* App is now fully initialised and inside the runloop. */
    for ( ;; )
    {
        next_timer = pdMS_TO_TICKS(1);
               
        if (!_app_shutting_down())
        {
            next_timer = appmanager_timer_get_next_expiry(_this_thread);
            
            if (next_timer < 0)
                next_timer = portMAX_DELAY;
        }

        /* we are inside the apps main loop event handler now */
        if (xQueueReceive(_app_message_queue, &data, next_timer))
        {
            /* We woke up for some kind of event that someone posted.  But what? */
            if (data.message_type_id == APP_BUTTON)
            {
                if (_app_shutting_down())
                    continue;
                
                if (overlay_window_accepts_keypress())
                {
                    overlay_window_post_button_message(data.payload);
                    continue;
                }
                /* execute the button's callback */
                ButtonMessage *message = (ButtonMessage *)data.payload;
                ((ClickHandler)(message->callback))((ClickRecognizerRef)(message->clickref), message->context);
            }
            /* Someone has requested the application close.
             * We will attempt graceful shutdown by unsubscribing timers
             * Any app timers will fire and be nulled, or get erased.
             * XXX could do with a timer mutex to wait on
             */
            else if (data.message_type_id == APP_QUIT)
            {
                /* Set the shutdown time for this app. We will kill it then */
                if (!_app_shutting_down())
                {
                    _this_thread->shutdown_at_tick = xTaskGetTickCount() + pdMS_TO_TICKS(5000);
                    _this_thread->status = AppThreadUnloading;
                }

                /* remove all of the clck handlers */
                button_unsubscribe_all();

                /* remove the ticktimer service handler and stop it */
                tick_timer_service_unsubscribe();

                /* Check we are not doing anything important
                 * If we are, then defer the render */
                if (!display_buffer_lock_take(0))
                {
                    _this_thread->status = AppThreadUnloading;
                    appmanager_app_quit();
                    continue;
                }
                else
                {
                    display_buffer_lock_give();
                }
                
                KERN_LOG("app", APP_LOG_LEVEL_INFO, "App Quit");

                /* app was quit, break out of this loop into the main handler */
                break;
            }

            /* A draw is requested. Get a lock and then draw. if we can't lock we..
             * try, try, try again
             */
            else if (data.message_type_id == APP_DRAW)
            {
                if (_app_shutting_down())
                    continue;
                
                /* Request a draw. This is mostly from an app invalidating something */
                if (display_buffer_lock_take(0))
                {
                    window_draw();
                    if (overlay_window_count() > 0)
                        overlay_window_draw(true);
                    
                    display_draw();
                    display_buffer_lock_give();
                }
                else
                {
                    /* We didn't get the mutex. Set a flag for when the draw completes */
                    KERN_LOG("app", APP_LOG_LEVEL_INFO, "draw deferred");
                    appmanager_post_draw_message(0);
                }
                continue;
            }
        } else {
            if (_app_shutting_down())
                continue;

            appmanager_timer_expired(_this_thread);
            /* Something changed, lets see if we can draw */
            appmanager_post_draw_message(0);
        }
    }
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "App Signalled shutdown...");
    /* We fall out of the apps main_ now and into deinit and thread completion
     * We will hand back control to appmanager_app_main_entry above */
}

/* Timer util */
TickType_t appmanager_timer_get_next_expiry(app_running_thread *thread)
{
    TickType_t next_timer;

    if (thread->timer_head) {
        TickType_t curtime = xTaskGetTickCount();
        if (curtime > thread->timer_head->when) {
            next_timer = 0;
        }
        else
            next_timer = thread->timer_head->when - curtime;
    } else {
        next_timer = portMAX_DELAY; /* Just block forever. */
    }

    return next_timer;
}

void appmanager_timer_expired(app_running_thread *thread)
{
    /* We woke up because we hit a timer expiry.  Dequeue first,
     * then invoke -- otherwise someone else could insert themselves
     * at the head, and we would wrongfully dequeue them!  */
    assert(thread);
    CoreTimer *timer = thread->timer_head;
    assert(timer);
    
    if (!timer->callback) {
        /* assert(!"BAD"); // actually this is pretty bad. I've seen this 
         * happen only once before when the app draw was happening while the
         * ovelay thread was coming up. The ov thread memory was memset to 0. */
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Bad Callback!");
        thread->timer_head = timer->next;
        return;
    }

    thread->timer_head = timer->next;
    
    app_running_thread *_this_thread = appmanager_get_current_thread();
    if (!_app_shutting_down())
        timer->callback(timer);
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
