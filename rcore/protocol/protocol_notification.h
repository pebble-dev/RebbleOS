#pragma once

#include "node_list.h"


typedef struct {
    uint8_t unk0;
    uint8_t add_nofif;
    uint32_t flags;
    uint32_t id;
    uint32_t ancs_id;
    uint32_t ts;
    uint8_t layout;
    uint8_t attr_count;
    uint8_t action_count;
    // These follow in the buffer
    //cmd_phone_attribute *attributes;
    //cmd_phone_attribute *actions;
} __attribute__((__packed__)) cmd_phone_notify_t;



typedef struct {
    uint8_t attr_idx;
    uint16_t str_len;
} __attribute__((__packed__)) cmd_phone_attribute_hdr_t;

typedef struct {
    cmd_phone_attribute_hdr_t hdr;
    list_node node;
    uint8_t *data;
} __attribute__((__packed__)) cmd_phone_attribute_t;


typedef struct {
    uint8_t id;
    uint8_t cmd_id;
    uint8_t attr_count;
    uint8_t attr_id;
    uint16_t str_len;
} __attribute__((__packed__)) cmd_phone_action_hdr_t;

typedef struct {
    cmd_phone_action_hdr_t hdr;
    list_node node;
    uint8_t *data;
} __attribute__((__packed__)) cmd_phone_action_t;






typedef struct full_msg {
    cmd_phone_notify_t *header;
    list_head attributes_list_head;
    list_head actions_list_head;
    list_node node;
} full_msg_t;


full_msg_t *notification_get(void);
void process_notification_packet(uint8_t *data);
void notification_packet_push(uint8_t *data, full_msg_t **message);
