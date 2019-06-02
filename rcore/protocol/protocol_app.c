/* protocol_app.c
 * Protocol handlers for application messages over the wire
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include <stdlib.h>
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"

enum {
    AppRunStateStart = 1,
    AppRunStateStop = 2,
    AppRunStateRequest = 3,
};


static void _app_send_state(const pbl_transport_packet *packet)
{
    uint8_t resp[sizeof(Uuid) + 1];
    resp[0] = AppRunStateStart;
    SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "App State");
    App *app = appmanager_get_current_app();
    SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "App State %x %x", app, app->header->uuid);
    if (app)
    {
        memcpy(&resp[1], &app->header->uuid, sizeof(Uuid));
    }
    pbl_transport_packet pkt = {
        .length = sizeof(Uuid) + 1,
        .endpoint = WatchProtocol_AppRunState,
        .data = (uint8_t *)resp,
        .transport_sender = packet->transport_sender
    };
    protocol_send_packet(&pkt);
}


static void _app_start_app(const pbl_transport_packet *packet)
{
    
}

static void _app_stop_app(const pbl_transport_packet *packet)
{
    
}


void protocol_app_run_state(const pbl_transport_packet *packet)
{
    switch (packet->data[0]) {
        case AppRunStateRequest:
            _app_send_state(packet);
            break;
        case AppRunStateStart:
            _app_start_app(packet);
            break;
        case AppRunStateStop:
            _app_stop_app(packet);
            break;
        default:
            assert(!"IMPLEMENT ME");
    }
    
}

typedef struct _app_fetch_t {
    uint8_t command;
    Uuid uuid;
    int32_t app_id;
} __attribute__((__packed__)) _app_fetch;

typedef struct _app_fetch_response_t {
    uint8_t command;
    uint8_t response;
} __attribute__((__packed__)) _app_fetch_response;

enum {
    AppFetchStatusStart = 0x01,
    AppFetchStatusBusy = 0x02,
    AppFetchStatusInvalidUUID = 0x03,
    AppFetchStatusNoData = 0x04,
};

void protocol_app_fetch(const pbl_transport_packet *packet)
{
    _app_fetch *app = (_app_fetch *)packet->data;
    _app_fetch_response resp = {
        .command = 1,
        .response = AppFetchStatusStart,
    };
    pbl_transport_packet pkt = {
        .length = sizeof(_app_fetch_response),
        .endpoint = WatchProtocol_AppFetch,
        .data = (uint8_t *)&resp,
        .transport_sender = packet->transport_sender
    };
    protocol_send_packet(&pkt);
}
