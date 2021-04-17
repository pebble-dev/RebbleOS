/* event_service.c
 ...
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol.h"
#include "pebble_protocol.h"
#include "overlay_manager.h"
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

// static void _event_subscriber_insert(event_service_subscriber *conn)
// {
//     // subs in the main app thread are first
//     // subs in the overlay thread after
//     list_init_node(&conn->node);

//     if (&conn->thread->thread_type == AppThreadMainApp)
//     {
//         // list_insert_before(&_subscriber_list_head, &_subscriber_list_head.node, &conn->node);
//         list_insert_head(&_subscriber_list_head, &conn->node);
//     }
//     else
//     {
//         list_insert_before(&_subscriber_list_head, &_subscriber_list_head.node, &conn->node);
//     }
// }

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

void event_service_unsubscribe_thread(EventServiceCommand command, app_running_thread *thread)
{
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->thread == thread)
        {
            list_remove(&_subscriber_list_head, &conn->node);
            remote_free(conn);
            break;
        }
    }
}

void event_service_unsubscribe_thread_all(app_running_thread *thread)
{
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->thread == thread)
        {
            list_remove(&_subscriber_list_head, &conn->node);
            remote_free(conn);
            break;
        }
    }
}

void event_service_unsubscribe(EventServiceCommand command)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_unsubscribe_thread(command, _this_thread);
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

bool event_service_post(EventServiceCommand command, void *data, DestroyEventProc destroy_callback)
{
    /* Step 1. post to the app thread */
    if (appmanager_post_event_message(command, data, destroy_callback) == false)
    {
        LOG_ERROR("Queue Full! Not processing");
        destroy_callback(data);
        return false;
    }
    
    return true;
}

void event_service_event_trigger(EventServiceCommand command, void *data, DestroyEventProc destroy_callback)
{
    DestroyEventProc destroy = destroy_callback;
    
    if (destroy_callback)
        destroy = MK_THUMB_CB(destroy_callback);
    
    app_running_thread *_this_thread = appmanager_get_current_thread();
    event_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, event_service_subscriber, node)
    {
        if (conn->command == command)
        {
            LOG_DEBUG(
                "Checking %s subscriber %x for event type %x",
                conn->thread->thread_type == AppThreadMainApp ? "main" : "overlay",
                conn,
                command);

            /* Step 1. Invoke the subscriber callback - this can either
             * be a handler on the app or overlay thread */
            if (conn->thread == _this_thread && conn->callback)
            {
                LOG_DEBUG("Triggering callback %x with %x %x", conn->callback, command, data);
                conn->callback(command, data, conn->context);
            }
        }
    }

    /* Step 2. Subscriber is done, post it to overlay thread if this is the main thread */
    if (_this_thread->thread_type == AppThreadMainApp)
    {
        LOG_DEBUG("Posting event to overlay thread %x %x %x", command, data, destroy);
        overlay_window_post_event(command, data, destroy);
    }
        
    /* Step 3. if we are the overlay thread, then destroy this packet */
    else if (_this_thread->thread_type == AppThreadOverlay)
    {
        if (destroy)
        {
            LOG_DEBUG("Destroying event packet data %x", data);
            destroy(data);
        }

        appmanager_post_draw_message(1);
    }
}
