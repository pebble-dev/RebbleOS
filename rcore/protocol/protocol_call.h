#pragma once

typedef struct phone_message_t {
    uint8_t command_id;
    uint32_t cookie;
    uint8_t pascal_string_data[];
}  __attribute__((__packed__)) phone_message;

typedef struct rebble_phone_message_t {
    phone_message phone_message;
    char *number;
    char *name;
} rebble_phone_message;

enum {
    PhoneMessage_AnswerCall         = 0x01,
    PhoneMessage_HangupCall         = 0x02,
    PhoneMessage_PhoneStateRequest  = 0x03,
    PhoneMessage_PhoneStateResponse = 0x83,
    PhoneMessage_IncomingCall       = 0x04,
    PhoneMessage_OutgoingCall       = 0x05,
    PhoneMessage_MissedCall         = 0x06,
    PhoneMessage_Ring               = 0x07,
    PhoneMessage_CallStart          = 0x08,
    PhoneMessage_CallEnd            = 0x09,
};


void protocol_phone_message_process(const RebblePacket packet);
rebble_phone_message *protocol_phone_create(uint8_t *data, uint16_t length);
void protocol_phone_destroy(rebble_phone_message *msg);

/* Client API */
void protocol_phone_answer();
void protocol_phone_hangup();
void protocol_phone_get_state();
void protocol_phone_message_send(uint8_t command_id, uint32_t cookie, bool needs_ack);
