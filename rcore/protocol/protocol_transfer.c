/* protocol_transfer.c
 * Protocol handlers putting and getting binary bytes to the device
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
/*
 * Bytes are downloaded to the device with this sequence
 * init (size), put, commit, install
 * 
 * Apps are downloaded to the device in this order
 *  Binary, Resource, Worker
 *  XXX Worker is not currently supported
 * 
 *  XXX todo failed transfer mid way
 *   timeout
 *   cleanup
 * 
 * Each request requires an (N)ACK
 */
#include <stdlib.h>
#include "rebbleos.h"
#include "protocol_system.h"
#include "pebble_protocol.h"
#include "protocol_service.h"
#include "notification_manager.h"

/* Configure Logging */
#define MODULE_NAME "pcolxfr"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

static uint8_t _transfer_state = 0;
static size_t _total_size = 0;
static size_t _bytes_transferred = 0;
static uint32_t _cookie = 0;
static uint8_t _transfer_type;
static struct fd _fd;


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

static void _send_xack(const RebblePacket packet, uint8_t ack, uint32_t cookie)
{
    _transfer_put_response resp = {
        .result = ack,
        .cookie = cookie
    };
    
    packet_reply(packet, (void *)&resp, sizeof(_transfer_put_response));
}

static inline void _send_ack(const RebblePacket packet, uint32_t cookie)
{
    _send_xack(packet, ACK, cookie);
}

static inline void _send_nack(const RebblePacket packet, uint32_t cookie)
{
    _send_xack(packet, NACK, cookie);
}

void protocol_process_transfer(const RebblePacket packet)
{
     uint8_t *data = packet_get_data(packet);

     switch (data[0]) {
        case PutBytesInit:
            if (_transfer_state != 0) {
                LOG_ERROR("Invalid state for receiving data");
                goto error;
            }
            if (protocol_transaction_lock(200) < 0) {
                LOG_ERROR("failed to acquire bulk xfer transaction lock");
                goto error;
            }

            _transfer_put_app_init_header *hdr = (_transfer_put_app_init_header *)data;
            LOG_INFO("PUT INIT cmd %d, sz %d t %d id %d", hdr->command, ntohl(hdr->total_size), hdr->data_type, ntohl(hdr->app_id));
            _total_size = ntohl(hdr->total_size);
            _transfer_state = PutBytesInit;
            
            char buf[16];
            char sel[6];
            _transfer_type = hdr->data_type & 0x7F;
            
            if (_transfer_type == TransferType_AppExecutable)
                snprintf(sel, 4, "app");
            else if (_transfer_type == TransferType_AppResource)
                snprintf(sel, 4, "res");
            else
                snprintf(sel, 4, "fle"); //??
            
            snprintf(buf, 16, "@%08x/%s", ntohl(hdr->app_id), sel);
            
            struct file file;
            if (fs_find_file(&file, buf) >= 0)
            {
                /* XXX delete it? */
                LOG_ERROR("File %s already exists. Fail!", buf);
                goto error;
            }
                        
            if (fs_creat(&_fd, buf, _total_size) == NULL) {
                LOG_ERROR("Couldn't create %s!", buf);
                goto error;
            }
            _send_ack(packet, 0);
            break;
            
        case PutBytesTransfer:
            LOG_DEBUG("Data Transfer Started");
            _transfer_put_header *nhdr = (_transfer_put_header *)data;
            size_t data_size = ntohl(nhdr->data_size);
            
            if (_transfer_state != PutBytesInit)
            {
                LOG_ERROR("Invalid state for receiving data");
                goto error;
            }
            LOG_DEBUG("PUT DATA cmd %d, cookie %d sz %d", nhdr->command, nhdr->cookie, data_size);
            
            if (fs_write(&_fd, nhdr->data, data_size) < data_size) {
                LOG_ERROR("failed to write data");
                goto error;
            }
            _cookie = nhdr->cookie;
            _bytes_transferred += data_size;
            
            notification_progress *prog = mem_heap_alloc(&mem_heaps[HEAP_LOWPRIO], sizeof(notification_progress));
            if (prog) {
                prog->progress_bytes = _bytes_transferred;
                prog->total_bytes = _total_size;
            
                event_service_post(EventServiceCommandProgress, prog, remote_free);
                vTaskDelay(0);
            }
            _send_ack(packet, nhdr->cookie);
            break;
            
        case PutBytesCommit:
            LOG_DEBUG("Commit %d Bytes", _total_size);
            _transfer_put_commit_header *chdr = (_transfer_put_commit_header *)data;
            _cookie = chdr->cookie;
            
            if (_bytes_transferred != _total_size)
            {
                LOG_ERROR("Not enough data arrived %d/%d", _bytes_transferred, _total_size);
                goto error;
            }
            
            /* CRC the file */
            uint32_t crc = fs_file_crc32(&_fd, _bytes_transferred);
            
            if (htonl(chdr->crc) != crc)
            {
                LOG_ERROR("Bad CRC: %x expected %x", crc, htonl(chdr->crc));
                goto error;
            }

            fs_mark_written(&_fd);
            LOG_DEBUG("CRC %x valid", crc);
            
            _send_ack(packet, _cookie);
            break;
        
        case PutBytesInstall:
            LOG_INFO("Install App");
            _transfer_put_install_header *ihdr = (_transfer_put_install_header *)data;           
            _cookie = ihdr->cookie;
            _transfer_state = 0;
            _send_ack(packet, _cookie);
            _bytes_transferred = 0;
            _cookie = 0;
            
            protocol_transaction_unlock();
            
            /* Once we have the resource, tell app manager we are good to load */
            if (_transfer_type == TransferType_AppResource)
                appmanager_app_download_complete();
            
            break;
            
        case PutBytesAbort:
            LOG_INFO("Transfer Aborted");
            goto error;
            
        default:
            assert(!"IMPLEMENT ME");
    }
    return;
    
error:
    _transfer_state = 0;
    _bytes_transferred = 0;
    _send_nack(packet, 0);
    protocol_transaction_unlock();
}
