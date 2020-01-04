/* routines for Managing the Connection State Event Service
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "node_list.h"
#include "event_service.h"
#include "event_service.h"
#include "connection_service.h"


static void _connection_service_cb(EventServiceCommand command, void *data, void *context)
{
    /* context contains the handlers */
    ConnectionHandlers *handlers = (ConnectionHandlers *)context;
    
    if (handlers->pebble_app_connection_handler)
        handlers->pebble_app_connection_handler((uint32_t)data);
}

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
    ConnectionHandlers *handlers = app_calloc(1, sizeof(ConnectionHandlers));
    handlers->pebble_app_connection_handler = conn_handlers.pebble_app_connection_handler;
    handlers->pebblekit_connection_handler = conn_handlers.pebblekit_connection_handler;

    if (handlers->pebble_app_connection_handler)
        MK_THUMB_CB(handlers->pebble_app_connection_handler);

    if (handlers->pebblekit_connection_handler)
        MK_THUMB_CB(handlers->pebblekit_connection_handler);
    
    event_service_subscribe_with_context(EventServiceCommandConnectionService, _connection_service_cb, handlers);
}

void connection_service_unsubscribe(void)
{
    void *context = event_service_get_context(EventServiceCommandConnectionService);
    if (context)
        app_free(context);
    event_service_unsubscribe(EventServiceCommandConnectionService);
}

void connection_service_update(bool connected)
{
    event_service_post(EventServiceCommandConnectionService, (void *)connected, NULL);
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
