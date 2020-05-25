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
void protocol_set_current_transport_sender(ProtocolTransportSender sender);
int protocol_parse_packet(uint8_t *data, RebblePacketDataHeader *packet, ProtocolTransportSender transport);

/* API */

void protocol_send_packet(const RebblePacket packet);

typedef enum  {
    Unknown          = 0x00,
    TintinBlack      = 0x01,
    TintinWhite      = 0x02,
    TintinRed        = 0x03,
    TintinOrange     = 0x04,
    TintinGrey       = 0x05,
    BiancaSilver     = 0x06,
    BiancaBlack      = 0x07,
    TintinBlue       = 0x08,
    TintinGreen      = 0x09,
    TintinPink       = 0x0a,
    SnowyWhite       = 0x0b,
    SnowyBlack       = 0x0c,
    SnowyRed         = 0x0d,
    BobbySilver      = 0x0e,
    BobbyBlack       = 0x0f,
    BobbyGold        = 0x10,
    Bobby1           = 0x11,
    Bobby2           = 0x12,
    Unk0             = 0x13,
    Unk1             = 0x14,
    Unk2             = 0x15,
    Unk3             = 0x16,
    Unk4             = 0x17,
    Unk5             = 0x18,
    SilkBlack        = 0x19,
    Unk6             = 0x1a,
    Unk7             = 0x1b,
    SilkRed          = 0x1c,
    Unk8             = 0x1d,
    Unk9             = 0x1e,
    EndPebbleDevice  = 0x1f,

    AsterixWhite     = 0x20,
    EndDevices       = 0x21
} WatchModel;

#define WATCH_MODEL SnowyBlack



void protocol_app_run_state(const RebblePacket packet);
void protocol_app_fetch(const RebblePacket packet);
void protocol_app_fetch_request(Uuid *uuid, uint32_t app_id);

void protocol_process_blobdb(const RebblePacket packet);
void protocol_process_timeline_action_response(const RebblePacket packet);

void protocol_process_transfer(const RebblePacket packet);
void protocol_app_fetch_request(Uuid *uuid, uint32_t app_id);
void protocol_process_reorder(const RebblePacket packet);

void protocol_process_appmessage(const RebblePacket *packet);


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
