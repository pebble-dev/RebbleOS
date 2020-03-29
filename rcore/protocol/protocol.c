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

int protocol_rx_buffer_append(uint8_t *data, size_t len)
{
    if (_buf_ptr + len > RX_BUFFER_SIZE)
        return PROTOCOL_BUFFER_FULL;

    _is_rx_buf_expired();
    _last_rx = xTaskGetTickCount();
    memcpy(&_rx_buffer[_buf_ptr], data, len);

    _buf_ptr += len;

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
    _is_rx_buf_expired();
    return &_rx_buffer[_buf_ptr];
}

void protocol_rx_buffer_release(uint16_t len)
{
    _last_rx = xTaskGetTickCount();
    _buf_ptr += len;
}

int protocol_rx_buffer_consume(uint16_t len)
{
    _last_rx = xTaskGetTickCount();
    uint16_t mv = _buf_ptr - len < 0 ? _buf_ptr : len;
    memmove(_rx_buffer, _rx_buffer + mv, _buf_ptr - mv);
    _buf_ptr -= mv;

    return mv;
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
    uint16_t pkt_length = (data[0] << 8) | (data[1] & 0xff);
    uint16_t pkt_endpoint = (data[2] << 8) | (data[3] & 0xff);
    
    _last_transport_used = transport;
    
    LOG_INFO("RX: %d/%d bytes to endpoint %04x", _buf_ptr, pkt_length + 4, pkt_endpoint);
    
    if (_buf_ptr < 4) /* not enough data to parse a header */
        return PACKET_MORE_DATA_REQD;

    /* done! (usually) */
    if (pkt_length == 0)
        return PACKET_PROCESSED;

    /* Seems sensible */
    if (pkt_length > RX_BUFFER_SIZE)
    {
        LOG_ERROR("RX: payload length %d. Seems suspect!", pkt_length);
        return PACKET_INVALID;
    }

    if (_buf_ptr < pkt_length + 4)
    {
        LOG_INFO("RX: Partial. Still waiting for %d bytes", (pkt_length + 4) - _buf_ptr);
        return PACKET_MORE_DATA_REQD;
    }

    LOG_INFO("RX: packet is complete %x", transport);

    /* it's a valid packet. fill out passed packet and finish up */
    packet->length = pkt_length;
    packet->endpoint = pkt_endpoint;
    packet->data = data + 4;
    packet->transport_sender = transport;

    return PACKET_PROCESSED;
}

/* 
 * Given a packet, process it and call the relevant function
 */
int protocol_process_packet(const RebblePacket packet)
{
    EndpointHandler handler = protocol_find_endpoint_handler(packet_get_endpoint(packet), protocol_get_pebble_endpoints());
    if (handler == NULL)
    {
        return PACKET_INVALID;
    }

    handler(packet);
    return PACKET_PROCESSED;
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



uint8_t pascal_string_to_string(uint8_t *result_buf, uint8_t *source_buf)
{
    uint8_t len = (uint8_t)source_buf[0];
    /* Byte by byte copy the src to the dest */
    for(int i = 0; i < len; i++)
        result_buf[i] = source_buf[i+1];
    
    /* and null term it */
    result_buf[len] = 0;
    
    return len + 1;
}

uint8_t pascal_strlen(char *str)
{
    return (uint8_t)str[0];
}
