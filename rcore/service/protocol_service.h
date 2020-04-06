#pragma once
typedef struct rebble_packet rebble_packet;

void rebble_protocol_init();
RebblePacket packet_create(uint16_t endpoint, uint16_t size);
RebblePacket packet_create_with_data(uint16_t endpoint, uint8_t *data, uint16_t length);
void packet_destroy(RebblePacket packet);
int packet_send(const RebblePacket packet);
int packet_reply(RebblePacket packet, uint8_t *data, uint16_t size);
int packet_recv(const RebblePacket packet);
uint8_t *packet_get_data(RebblePacket packet);
void packet_set_data(RebblePacket packet, void *data);
void packet_copy_data(RebblePacket packet, void *data, uint16_t size);
uint16_t packet_get_data_length(RebblePacket packet);
uint16_t packet_get_endpoint(RebblePacket packet);
void packet_set_endpoint(RebblePacket packet, uint16_t endpoint);
ProtocolTransportSender packet_get_transport(RebblePacket packet);
void packet_set_transport(RebblePacket packet, ProtocolTransportSender transport);
void packet_send_to_transport(RebblePacket packet, uint16_t endpoint, uint8_t *data, uint16_t len);