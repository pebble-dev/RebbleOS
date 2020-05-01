#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "uuid.h"

typedef void (*ProtocolTransportSender)(uint16_t endpoint, uint8_t *data, uint16_t len);

typedef struct rebble_packet * RebblePacket;

typedef struct rebble_packet_header_t {
    uint16_t length;
    uint16_t endpoint;
    uint8_t  data[];
} __attribute__((__packed__)) RebblePacketHeader;

typedef struct rebble_packet_data_header_t {
    uint16_t length;
    uint16_t endpoint;
    uint8_t  *data;
    ProtocolTransportSender transport_sender;
} __attribute__((__packed__)) RebblePacketDataHeader;

typedef void (*EndpointHandler)(const RebblePacket packet);

typedef struct
{
    uint16_t endpoint;
    EndpointHandler handler;
} PebbleEndpoint;

enum {
    ACK  = 0x1,
    NACK = 0x2
};

EndpointHandler protocol_find_endpoint_handler(uint16_t protocol, const PebbleEndpoint *endpoint);
PebbleEndpoint *protocol_get_pebble_endpoints(void);
void protocol_init(void);
uint8_t *protocol_get_rx_buffer(void);
int protocol_buffer_lock();
int protocol_buffer_unlock();
int protocol_transaction_lock(int ticks);
int protocol_transaction_unlock();
int protocol_rx_buffer_append(uint8_t *data, size_t len);
size_t protocol_get_rx_buf_size(void);
size_t protocol_get_rx_buf_free(void);
size_t protocol_get_rx_buf_used(void);
uint8_t *protocol_rx_buffer_request(void);
void protocol_rx_buffer_release(uint16_t len);
int protocol_rx_buffer_consume(uint16_t len);
int protocol_rx_buffer_pointer_adjust(int howmuch);
void protocol_rx_buffer_reset(void);
ProtocolTransportSender protocol_get_current_transport_sender();
int protocol_parse_packet(uint8_t *data, RebblePacketDataHeader *packet, ProtocolTransportSender transport);

/* API */

void protocol_send_packet(const RebblePacket packet);

typedef enum  {
    Unknown = 0,
    TintinBlack = 1,
    TintinWhite = 2,
    TintinRed = 3,
    TintinOrange = 4,
    TintinGrey = 5,
    BiancaSilver = 6,
    BiancaBlack = 7,
    TintinBlue = 8,
    TintinGreen = 9,
    TintinPink = 10,
    SnowyBlack = 11,
    SnowyWhite = 12,
    SnowyRed = 13,
    BobbySilver = 14,
    BobbyBlack = 15,
    BobbyGold = 16,
} WatchModel;

#define WATCH_MODEL SnowyBlack



void protocol_app_run_state(const RebblePacket packet);
void protocol_app_fetch(const RebblePacket packet);
void protocol_process_blobdb(const RebblePacket packet);
void protocol_process_timeline_action_response(const RebblePacket packet);
void protocol_process_transfer(const RebblePacket packet);
void protocol_app_fetch_request(Uuid *uuid, uint32_t app_id);

uint8_t pascal_string_to_string(uint8_t *result_buf, uint8_t *source_buf);
uint8_t pascal_strlen(char *str);

enum {
    PROTOCOL_BUFFER_FULL = -1,
    PROTOCOL_BUFFER_OK = 0,
    PACKET_MORE_DATA_REQD = -1,
    PACKET_BUFFER_HAS_DATA = -2,
    PACKET_INVALID = -3,
    PACKET_PROCESSED = 1,
};
