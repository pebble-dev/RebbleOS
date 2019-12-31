typedef struct rebble_packet rebble_packet;

void rebble_protocol_init();
void rebble_protocol_send(uint16_t endpoint, uint8_t *data, uint16_t len);
rebble_packet *rebble_protocol_get_awaiting_by_uuid(Uuid *uuid);
void rebble_protocol_remove_packet(rebble_packet *packet);


void protocol_free(void *mem);
void *protocol_calloc(uint8_t num, size_t size);

RebblePacket packet_create(uint16_t endpoint, uint16_t length);
RebblePacket packet_create_with_data(uint16_t endpoint, uint8_t *data, uint16_t length);
void packet_destroy(RebblePacket packet);
void packet_send(RebblePacket packet);
uint8_t *packet_get_data(RebblePacket packet);
void packet_set_data(RebblePacket packet, uint8_t *data);
void packet_set_retries(RebblePacket packet, uint8_t retries);
uint8_t packet_get_retries(RebblePacket packet);
void packet_set_timeout(RebblePacket packet, uint32_t timeout_ms);
uint32_t packet_get_timeout(RebblePacket packet);
void packet_set_ack_required(RebblePacket packet, Uuid *uuid);
uint16_t packet_get_data_length(RebblePacket packet);
void packet_set_endpoint(RebblePacket packet, uint16_t endpoint);
uint16_t packet_get_endpoint(RebblePacket packet);

void packet_send_to_transport(RebblePacket packet, uint16_t endpoint, uint8_t *data, uint16_t len);
