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

    rebble_protocol_send(WatchProtocol_AppRunState, &resp, sizeof(Uuid) + 1);
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

    rebble_protocol_send(WatchProtocol_AppFetch, &resp, sizeof(_app_fetch_response));
    packet_destroy(packet);
}

void protocol_app_fetch_request(Uuid *uuid, uint32_t app_id)
{
    _app_fetch req = {
        .command = AppFetchStatusStart,
        .app_id = app_id,
    };
    memcpy(&req.uuid, uuid, sizeof(Uuid));

    rebble_protocol_send(WatchProtocol_AppFetch, &req, sizeof(_app_fetch));    
}


enum {
    TransferType_Firmware        = 0x01,
    TransferType_Recovery        = 0x02,
    TransferType_SystemResource  = 0x03,
    TransferType_AppResource     = 0x04,
    TransferType_AppExecutable   = 0x05,
    TransferType_File            = 0x06,
    TransferType_Worker          = 0x07,
};

enum {
    PutBytesInit                 = 0x01,
    PutBytesTransfer             = 0x02,
    PutBytesCommit               = 0x03,
    PutBytesAbort                = 0x04,
    PutBytesInstall              = 0x05,
};    

typedef struct _transfer_header_t {
    uint8_t command;
    uint8_t data[];
} __attribute__((__packed__)) _transfer_header;

typedef struct _transfer_put_init_header_t {
    uint8_t command;
    uint32_t total_size;
    uint8_t data_type;
} __attribute__((__packed__)) _transfer_put_init_header;

typedef struct _transfer_put_app_init_header_t {
    uint8_t command;
    uint32_t total_size;
    uint8_t data_type;
    uint32_t app_id;
} __attribute__((__packed__)) _transfer_put_app_init_header;

typedef struct _transfer_put_header_t {
    uint8_t command;
    uint32_t cookie;
    uint32_t data_size;
    uint8_t data[];
} __attribute__((__packed__)) _transfer_put_header;

typedef struct _transfer_put_commit_header_t {
    uint8_t command;
    uint32_t cookie;
    uint32_t crc;
} __attribute__((__packed__)) _transfer_put_commit_header;

typedef struct _transfer_put_abort_header_t {
    uint8_t command;
    uint32_t cookie;
} __attribute__((__packed__)) _transfer_put_abort_header;

typedef struct _transfer_put_install_header_t {
    uint8_t command;
    uint32_t cookie;
} __attribute__((__packed__)) _transfer_put_install_header;


typedef struct _transfer_put_response_t {
    uint8_t result;
    uint32_t cookie;
} __attribute__((__packed__)) _transfer_put_response;

uint8_t transfer_state = 0;
uint32_t uploading_id = 0;
size_t total_size = 0;
uint32_t cookie = 0;
uint8_t transfer_type;
struct fd fd;
     
enum {
    ACK  = 0x1,
    NACK = 0x2
};

void protocol_process_transfer(const RebblePacket packet)
{
     uint8_t *data = packet_get_data(packet);
     struct file file;

     ApplicationHeader *header;
     _transfer_put_response resp = {
                .result = ACK,
                .cookie = cookie
            };
            
     switch (data[0]) {
        case PutBytesInit:
            if (transfer_state != 0)
            {
                LOG_ERROR("Invalid state for receiving data");
                goto error;
            }
            _transfer_put_app_init_header *hdr = (_transfer_put_app_init_header *)data;
            LOG_INFO("PUT INIT cmd %d, sz %d t %d id %d", hdr->command, ntohl(hdr->total_size), hdr->data_type, ntohl(hdr->app_id));
            total_size = ntohl(hdr->total_size);
            uploading_id = ntohl(hdr->app_id);
            transfer_state = PutBytesInit;
            
            char buf[16];
            char sel[6];
            transfer_type = hdr->data_type & 0x7F;
            
            if (transfer_type == TransferType_AppExecutable)
                snprintf(sel, 4, "app");
            else if (transfer_type == TransferType_AppResource)
                snprintf(sel, 4, "res");
            else
                snprintf(sel, 4, "fle"); //??
            
            snprintf(buf, 14, "@%08x/%s", uploading_id, sel);
            if (fs_creat(&fd, buf, total_size + sizeof(file_hdr)) == NULL) {
                LOG_ERROR("Couldn't create!");
                goto error;
            }
                    
            resp.result = ACK;
            resp.cookie = 0;
            
            rebble_protocol_send(WatchProtocol_PutBytes, &resp, sizeof(_transfer_put_response));
            break;
            
        case PutBytesTransfer:
            LOG_INFO("T");
            _transfer_put_header *nhdr = (_transfer_put_header *)data;
            size_t data_size = ntohl(nhdr->data_size);
            if (transfer_state != PutBytesInit)
            {
                LOG_ERROR("Invalid state for receiving data");
                goto error;
            }
            LOG_INFO("PUT DATA cmd %d, cookie %d sz %d", nhdr->command, ntohl(nhdr->cookie), ntohl(nhdr->data_size));
            
            if (fs_write(&fd, nhdr->data, data_size) < data_size) {
                LOG_ERROR("failed to write data");
                goto error;
            }          
            
            
            cookie = nhdr->cookie;

            resp.result = ACK;
            resp.cookie = nhdr->cookie;
            rebble_protocol_send(WatchProtocol_PutBytes, &resp, sizeof(_transfer_put_response));
            
            break;
            
        case PutBytesCommit:
            LOG_INFO("Commit Bytes");
                        
            resp.result = ACK;
            resp.cookie = cookie;
            rebble_protocol_send(WatchProtocol_PutBytes, &resp, sizeof(_transfer_put_response));
            
            break;
        case PutBytesInstall:
            LOG_INFO("Install App");
            
            if (transfer_type == TransferType_AppResource)
                appmanager_app_download_complete();
            
            transfer_state = 0;
            uploading_id = 0;
            resp.result = ACK;
            resp.cookie = cookie;
            rebble_protocol_send(WatchProtocol_PutBytes, &resp, sizeof(_transfer_put_response));
            cookie = 0;
            
            break;
        default:
            
            assert(!"IMPLEMENT ME");
    }
    packet_destroy(packet);
    return;
    
error:

    transfer_state = 0;
    uploading_id = 0;
    // XXX reply NACK
    
    resp.result = NACK;
    resp.cookie = 0;
    rebble_protocol_send(WatchProtocol_PutBytes, &resp, sizeof(_transfer_put_response));
    packet_destroy(packet);    
}
