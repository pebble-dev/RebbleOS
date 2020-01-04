/* event_service.c
 ...
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol.h"
#include "pebble_protocol.h"
#include "event_service.h"

/* Configure Logging */
#define MODULE_NAME "evtsvc"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static list_head _subscriber_list_head = LIST_HEAD(_subscriber_list_head);


typedef struct event_service_subscriber {
    EventServiceCommand command;
    EventServiceProc callback;
    void *context;
    app_running_thread *thread;
    list_node node;
} event_service_subscriber;

void event_service_subscribe(EventServiceCommand command, EventServiceProc callback)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn = app_calloc(1, sizeof(event_service_subscriber));
    conn->thread = _this_thread;
    conn->callback = MK_THUMB_CB(callback);
    conn->command = command;

    list_init_node(&conn->node);
    list_insert_head(&_subscriber_list_head, &conn->node);    
}

void event_service_subscribe_with_context(EventServiceCommand command, EventServiceProc callback, void *context)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn = app_calloc(1, sizeof(event_service_subscriber));
    conn->thread = _this_thread;
    conn->callback = MK_THUMB_CB(callback);
    conn->command = command;
    conn->context = context;

    list_init_node(&conn->node);
    list_insert_head(&_subscriber_list_head, &conn->node);    
}

void event_service_unsubscribe_all(void)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    bool done = false;
    
    while(!done)
    {
        done = true;
        event_service_subscriber *conn;
        list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
        {
            if (conn->thread == _this_thread)
            {
                list_remove(&_subscriber_list_head, &conn->node);
                app_free(conn);
                done = false;
                break;
            }
        }
    }
}

void event_service_unsubscribe(EventServiceCommand command)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->thread == _this_thread && conn->command == command)
        {
            list_remove(&_subscriber_list_head, &conn->node);
            app_free(conn);
            return;
        }
    }
}

void *event_service_get_context(EventServiceCommand command)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->thread == _this_thread && conn->command == command)
        {
            return conn->context;
        }
    }
    return NULL;
}

void event_service_set_context(EventServiceCommand command, void *context)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->thread == _this_thread && conn->command == command)
        {
            conn->context = context;
            return;
        }
    }
}

void event_service_post(EventServiceCommand command, void *data, DestroyEventProc destroy_callback)
{
    /* Step 1. post to the app thread */
    appmanager_post_event_message(command, data, destroy_callback);
}

void event_service_event_trigger(EventServiceCommand command, void *data, DestroyEventProc destroy_callback)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->command == command)
        {
            LOG_INFO("Triggering %x %x %x", data, destroy_callback, conn->callback);
            if (conn->thread == _this_thread && conn->callback)
                conn->callback(command, data, conn->context);
            
            /* Step 2. App processing done, post it to overlay thread */
            if (_this_thread->thread_type == AppThreadMainApp)
                overlay_window_post_event(command, data, destroy_callback);
                
            /* Step 3. if we are the overlay thread, then destroy this packet */
            else if (_this_thread->thread_type == AppThreadOverlay)
            {
                if (destroy_callback)
                {
                    destroy_callback = MK_THUMB_CB(destroy_callback);
                    destroy_callback(data);
                }
                appmanager_post_draw_message(1);
            }
        }
    }
}
