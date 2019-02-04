/* routines for Managing the Connection State Event Service
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "node_list.h"
#include "connection_service.h"

static list_head _subscriber_list_head = LIST_HEAD(_subscriber_list_head);

typedef struct connection_service_subscriber {
    ConnectionHandlers conn_handlers;
    app_running_thread *thread;
    list_node node;
} connection_service_subscriber;

bool connection_service_peek_pebble_app_connection(void)
{
    return bluetooth_is_device_connected();
}

bool connection_service_peek_pebblekit_connection(void)
{
    return false;
}

void connection_service_subscribe(ConnectionHandlers conn_handlers)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    connection_service_subscriber *conn = app_calloc(1, sizeof(connection_service_subscriber));
    conn->thread = _this_thread;
    conn->conn_handlers = conn_handlers;

    if (conn->conn_handlers.pebble_app_connection_handler)
        MK_THUMB_CB(conn->conn_handlers.pebble_app_connection_handler);

    if (conn->conn_handlers.pebblekit_connection_handler)
        MK_THUMB_CB(conn->conn_handlers.pebblekit_connection_handler);

    list_init_node(&conn->node);
    list_insert_head(&_subscriber_list_head, &conn->node);
}

void connection_service_unsubscribe(void)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    connection_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, connection_service_subscriber, node)
    {
        if (conn->thread == _this_thread)
        {
            list_remove(&_subscriber_list_head, &conn->node);
            app_free(conn);
        }
    }
}

void connection_service_update(bool connected)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    connection_service_subscriber *conn;
    list_foreach(conn, &_subscriber_list_head, connection_service_subscriber, node)
    {
        if (conn->thread == _this_thread &&
            conn->conn_handlers.pebble_app_connection_handler)
        {
                conn->conn_handlers.pebble_app_connection_handler(connected);
        }
    }
}

bool bluetooth_connection_service_peek(void)
{
    return connection_service_peek_pebble_app_connection();
}

void bluetooth_connection_service_subscribe(ConnectionHandler handler)
{
    connection_service_subscribe((ConnectionHandlers){ .pebble_app_connection_handler = handler });
}

void bluetooth_connection_service_unsubscribe(void)
{
    connection_service_unsubscribe();
}
