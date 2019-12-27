typedef struct rebble_packet rebble_packet;

void rebble_protocol_init();
void rebble_protocol_send(uint32_t endpoint, Uuid *uuid, void *data, size_t length, 
                          uint8_t retries, uint16_t timeout_ms, 
                          bool needs_ack) ;
rebble_packet *rebble_protocol_get_awaiting_by_uuid(Uuid *uuid);
void rebble_protocol_remove_packet(rebble_packet *packet);


void protocol_free(void *mem);
void *protocol_calloc(uint8_t num, size_t size);

