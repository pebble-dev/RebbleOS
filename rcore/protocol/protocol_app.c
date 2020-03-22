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
#include "protocol_service.h"

/* Configure Logging */
#define MODULE_NAME "pcolapp"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

enum {
    AppRunStateStart = 1,
    AppRunStateStop = 2,
    AppRunStateRequest = 3,
};

typedef struct app_start_t {
    uint8_t cmd;
    Uuid uuid;
    // and appid?
    uint32_t app_id;
} app_start;


static void _app_send_state(const RebblePacket packet)
{
    uint8_t resp[sizeof(Uuid) + 1];
    resp[0] = AppRunStateStart;
    LOG_INFO("App State");
    App *app = appmanager_get_current_app();
    LOG_INFO("App State %x %x", app, app->header->uuid);
    if (app)
    {
        memcpy(&resp[1], &app->header->uuid, sizeof(Uuid));
    }

    rebble_protocol_send(WatchProtocol_AppRunState, (void *)&resp, sizeof(Uuid) + 1);
}


static void _app_start_app(const RebblePacket packet)
{
    char buf[UUID_STRING_BUFFER_LENGTH];
    app_start *d = (app_start *)packet_get_data(packet);
    uuid_to_string(&d->uuid, buf);
    
    LOG_INFO("App Start %s", buf);
    appmanager_app_start_by_uuid(&d->uuid);
}

static void _app_stop_app(const RebblePacket packet)
{
    
}


void protocol_app_run_state(const RebblePacket packet)
{
    uint8_t *data = packet_get_data(packet);
    switch (data[0]) {
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
    packet_destroy(packet);
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

void protocol_app_fetch(const RebblePacket packet)
{
    uint8_t *data = packet_get_data(packet);
    _app_fetch *app = (_app_fetch *)data;
    _app_fetch_response resp = {
        .command = 1,
        .response = AppFetchStatusStart,
    };

    rebble_protocol_send(WatchProtocol_AppFetch, (void *)&resp, sizeof(_app_fetch_response));
    packet_destroy(packet);
}

void protocol_app_fetch_request(Uuid *uuid, uint32_t app_id)
{
    _app_fetch req = {
        .command = AppFetchStatusStart,
        .app_id = app_id,
    };
    memcpy(&req.uuid, uuid, sizeof(Uuid));

    rebble_protocol_send(WatchProtocol_AppFetch, (void *)&req, sizeof(_app_fetch));    
}
