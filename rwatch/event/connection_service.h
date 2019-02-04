#pragma once
typedef void(* ConnectionHandler)(bool connected);
typedef ConnectionHandler BluetoothConnectionHandler;

typedef struct ConnectionHandlers {
    ConnectionHandler pebble_app_connection_handler;
    ConnectionHandler pebblekit_connection_handler;
} ConnectionHandlers;

bool connection_service_peek_pebble_app_connection(void);
bool connection_service_peek_pebblekit_connection(void);
void connection_service_subscribe(ConnectionHandlers conn_handlers);
void connection_service_unsubscribe(void);
bool bluetooth_connection_service_peek(void);
void bluetooth_connection_service_subscribe(ConnectionHandler handler);
void bluetooth_connection_service_unsubscribe(void);
