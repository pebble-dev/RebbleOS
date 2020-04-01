/* protocol.c
 * Protocol processer
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include <stdlib.h>
#include "rebbleos.h"
#include "endpoint.h"
#include "protocol.h"
#include "pebble_protocol.h"
#include "protocol_notification.h"
#include "protocol_system.h"
#include "protocol_service.h"
#include "qemu.h"
#include "rtoswrap.h"

/* Configure Logging */
#define MODULE_NAME "pcol"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE


const PebbleEndpoint pebble_endpoints[] =
{
    { .endpoint = WatchProtocol_Time,               .handler  = protocol_time           },
    { .endpoint = WatchProtocol_FirmwareVersion,    .handler  = protocol_watch_version  },
    { .endpoint = WatchProtocol_WatchModel,         .handler  = protocol_watch_model    },
    { .endpoint = WatchProtocol_PingPong,           .handler  = protocol_ping_pong      },
    { .endpoint = WatchProtocol_AppVersion,         .handler  = protocol_app_version    },
    { .endpoint = WatchProtocol_AppRunState,        .handler  = protocol_app_run_state  },
    { .endpoint = WatchProtocol_AppFetch,           .handler  = protocol_app_fetch      },
    { .endpoint = WatchProtocol_Reset,              .handler  = protocol_watch_reset    },
    { .endpoint = WatchProtocol_LegacyMessage,      .handler  = protocol_process_legacy2_notification },
    { .endpoint = WatchProtocol_BlobDbMessage,      .handler  = protocol_process_blobdb },
    { .endpoint = WatchProtocol_PhoneMessage,       .handler  = protocol_phone_message_process },
    { .endpoint = WatchProtocol_MusicControl,       .handler  = protocol_music_message_process },
    { .endpoint = WatchProtocol_TimelineAction,     .handler  = protocol_process_timeline_action_response },
    { .endpoint = WatchProtocol_PutBytes,           .handler  = protocol_process_transfer },
    { .handler = NULL }
};

/*
const RebbleProtocol rebble_protocols[] =
{
    { .protocol = ProtocolBluetooth,                .handler  = protocol_time           },
    { .protocol = ProtocolQEmu,                     .handler  = protocol_time           },
};
*/

EndpointHandler protocol_find_endpoint_handler(uint16_t protocol, const PebbleEndpoint *endpoint)
{
    while (endpoint->handler != NULL)
    {
        if (endpoint->endpoint == protocol)
            return endpoint->handler;
        endpoint = (void *)endpoint + sizeof(PebbleEndpoint);
    }
    return NULL;
}

inline PebbleEndpoint *protocol_get_pebble_endpoints(void)
{
    return pebble_endpoints;
}


/*
 * Packet processing
 */
#define RX_BUFFER_SIZE 2048
static uint8_t _rx_buffer[RX_BUFFER_SIZE];
static uint16_t _buf_ptr = 0;
static TickType_t _last_rx;
static ProtocolTransportSender _last_transport_used;
MUTEX_DEFINE(rx_buf);

void protocol_init(void)
{
    MUTEX_CREATE(rx_buf);
}

uint8_t *protocol_get_rx_buffer(void)
{
    return _rx_buffer;
}

static void _is_rx_buf_expired(void)
{
    if (!_buf_ptr)
        return;
    TickType_t now = xTaskGetTickCount();
    if ((now - _last_rx) > pdMS_TO_TICKS(250))
    {
        LOG_ERROR("RX: Buffer timed out. Reset %d %d", now, _last_rx);
        _buf_ptr = 0;
    }
}

int protocol_buffer_lock()
{
    if (!xSemaphoreTake(MUTEX_HANDLE(rx_buf), 100)) 
        return -1;
    return 0;
}

int protocol_buffer_unlock()
{
    xSemaphoreGive(MUTEX_HANDLE(rx_buf));
    return 0;
}

int protocol_rx_buffer_append(uint8_t *data, size_t len)
{
    if (_buf_ptr + len > RX_BUFFER_SIZE)
        return PROTOCOL_BUFFER_FULL;

    _is_rx_buf_expired();
    _last_rx = xTaskGetTickCount();

    protocol_buffer_lock();
    memcpy(&_rx_buffer[_buf_ptr], data, len);
    _buf_ptr += len;
    protocol_buffer_unlock();

    return PROTOCOL_BUFFER_OK;
}

size_t protocol_get_rx_buf_size(void)
{
    return RX_BUFFER_SIZE;
}

size_t protocol_get_rx_buf_free(void)
{
    return RX_BUFFER_SIZE - _buf_ptr;
}

size_t protocol_get_rx_buf_used(void)
{
    return _buf_ptr;
}

uint8_t *protocol_rx_buffer_request(void)
{
    protocol_buffer_lock();
    _is_rx_buf_expired();
    uint8_t *b = &_rx_buffer[_buf_ptr];
    protocol_buffer_unlock();
    
    return b;
}

void protocol_rx_buffer_release(uint16_t len)
{
    _last_rx = xTaskGetTickCount();
    protocol_buffer_lock();
    _buf_ptr += len;
    protocol_buffer_unlock();
}

int protocol_rx_buffer_consume(uint16_t len)
{
    _last_rx = xTaskGetTickCount();
    protocol_buffer_lock();
    uint16_t mv = _buf_ptr - len < 0 ? _buf_ptr : len;
    memmove(_rx_buffer, _rx_buffer + mv, _buf_ptr - mv);
    _buf_ptr -= mv;
    protocol_buffer_unlock();
    return mv;
}

int protocol_rx_buffer_pointer_adjust(int howmuch)
{
    /* XXX not locked. kinda don't use this function unless you know why you need to */
    _buf_ptr += howmuch;

    return howmuch;
}

void protocol_rx_buffer_reset(void)
{
    _buf_ptr = 0;
}

ProtocolTransportSender protocol_get_current_transport_sender()
{
    return _last_transport_used ? _last_transport_used : qemu_send_data;
}

/*
 * Parse a packet in the buffer. Will fill the given pbl_transport with
 * the parsed data
 * 
 * Returns false when done or on completion errors
 */
int protocol_parse_packet(uint8_t *data, RebblePacketDataHeader *packet, ProtocolTransportSender transport)
{
    int rv = 0;
    uint16_t pkt_length = (data[0] << 8) | (data[1] & 0xff);
    uint16_t pkt_endpoint = (data[2] << 8) | (data[3] & 0xff);
    
    _last_transport_used = transport;
    
    protocol_buffer_lock();
    LOG_INFO("RX: %d/%d bytes to endpoint %04x", _buf_ptr, pkt_length + 4, pkt_endpoint);
    
    if (_buf_ptr < 4) { /* not enough data to parse a header */
        rv = PACKET_MORE_DATA_REQD;
        goto done;
    }

    /* done! (usually) */
    if (pkt_length == 0) {
        rv = PACKET_PROCESSED;
        goto done;
    }

    /* Seems sensible */
    if (pkt_length > RX_BUFFER_SIZE) {
        LOG_ERROR("RX: payload length %d. Seems suspect!", pkt_length);
        rv = PACKET_INVALID;
        goto done;
    }

    if (_buf_ptr < pkt_length + 4) {
        LOG_INFO("RX: Partial. Still waiting for %d bytes", (pkt_length + 4) - _buf_ptr);
        rv = PACKET_MORE_DATA_REQD;
        goto done;
    }

    LOG_INFO("RX: packet is complete %x", transport);

    /* it's a valid packet. fill out passed packet and finish up */
    packet->length = pkt_length;
    packet->endpoint = pkt_endpoint;
    packet->data = data + 4;
    packet->transport_sender = transport;

    rv = PACKET_PROCESSED;

done:
    protocol_buffer_unlock();
    return rv;
}

/*
 * Send a Pebble packet right now
 */
void protocol_send_packet(const RebblePacket packet)
{
    uint16_t len = packet_get_data_length(packet);
    uint16_t endpoint = packet_get_endpoint(packet);

    LOG_DEBUG("TX protocol: e:%d l %d", endpoint, len);
    _last_transport_used = packet_get_transport(packet);

    packet_send_to_transport(packet, endpoint, packet_get_data(packet), len);
}

#ifdef REBBLEOS_TESTING
#include "test.h"
extern const PebbleEndpoint qemu_endpoints[];
uint8_t _test_reply_buf[32];

TEST(protocol_basic) {
    uint8_t buf = 0;
    protocol_rx_buffer_reset();

    LOG_INFO("protocol test buffer greedy");
    if (protocol_rx_buffer_append(&buf, 1) == PROTOCOL_BUFFER_OK) {
        int mv = protocol_rx_buffer_consume(2);
        if (protocol_rx_buffer_consume(2) != 0) {
            LOG_ERROR("failed to append 1 remove 2 %d", mv);
            goto fail;
        }

        protocol_rx_buffer_reset();
    }
    
    LOG_INFO("protocol test buffer fill");
    int remaining = protocol_get_rx_buf_size();
    while (remaining) {        
        buf = remaining % 0xFF;
        if (protocol_rx_buffer_append((uint8_t *)&buf, 1) != PROTOCOL_BUFFER_OK) {
            LOG_ERROR("Could not append byte %d, %d", buf, _buf_ptr);
            goto fail;
        }
        remaining--;
    }

    LOG_INFO("protocol test buffer overfill");
    if (protocol_get_rx_buf_used() != protocol_get_rx_buf_size()) {
        LOG_ERROR("Buffer not full after fill");
        goto fail;
    }
    buf = 1;
    if (protocol_rx_buffer_append(&buf, 1) != PROTOCOL_BUFFER_FULL) {
        LOG_ERROR("Buffer should be full %d, %d", buf, _buf_ptr);
        goto fail;
    }

    LOG_INFO("protocol test buffer consume");
    protocol_rx_buffer_consume(8);
    uint8_t *pbuf = protocol_get_rx_buffer();

    if (protocol_get_rx_buf_used() + 8 != protocol_get_rx_buf_size()) {
        LOG_ERROR("Buffer used invalid");
        goto fail;
    }

    if (pbuf[0] != 0 && pbuf[1] != 254) {
        LOG_ERROR("Buffer contents(%d) invalid", pbuf[0]);
        goto fail;
    }
    
   
    *artifact = 0;
    return TEST_PASS;

fail:
    protocol_rx_buffer_reset();
    return TEST_FAIL;
}

TEST(protocol_packet) {
    uint8_t *pbuf = protocol_get_rx_buffer();
    uint8_t pacbuf[5];

    LOG_INFO("protocol test time message");
    protocol_rx_buffer_reset();
    RebblePacketDataHeader hdr = {
        .length = htons(1),
        .endpoint = htons(WatchProtocol_Time),
    };

    protocol_rx_buffer_append((uint8_t *)&hdr, 4);
    pacbuf[0] = 0;
    memset(_test_reply_buf, 0, sizeof(_test_reply_buf));
    protocol_rx_buffer_append(pacbuf, 1);
    EndpointHandler handler = protocol_find_endpoint_handler(QemuProtocol_TestsLoopback, qemu_endpoints);
    assert(handler);
    RebblePacket p = packet_create_with_data(2, pbuf, 1);
    handler(p);

    /* test the packet */
    if (_test_reply_buf[0] != 1) { /* Time Response endpoint */
        LOG_ERROR("Packet endpoint invalid(%d)", (uint8_t *)_test_reply_buf[0]);
        return TEST_FAIL;
    }
    uint32_t t = *(uint32_t *)&_test_reply_buf[1];
    if (htonl(t) != (uint32_t)rcore_get_time()) { /* time back */
        LOG_ERROR("Packet time invalid(%x) now %x", htonl(t), (uint32_t)rcore_get_time());
        return TEST_FAIL;
    }

    *artifact = 0;
    return TEST_PASS;
}

void test_packet_loopback_sender(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    memcpy(_test_reply_buf, data, len);
    KERN_LOG("test", APP_LOG_LEVEL_INFO, "test RX got %d bytes %x %x %x %x %x", len, data[0], data[1], data[2], data[3], data[4]);
}

void test_packet_loopback_handler(const RebblePacket packet)
{
    ProtocolTransportSender _default_handler = _last_transport_used;
    
    KERN_LOG("test", APP_LOG_LEVEL_INFO, "test RX got %d bytes", packet_get_data_length(packet));
    _last_transport_used = test_packet_loopback_sender;
    packet_set_transport(packet, test_packet_loopback_sender);
    spp_handler(packet);
    _last_transport_used = _default_handler;
}
#endif