#pragma once

#include "node_list.h"
#include "protocol.h"
#include "endpoint.h"


// typedef struct full_msg {
//     cmd_phone_notify_t *header;
//     list_head attributes_list_head;
//     list_head actions_list_head;
//     list_node node;
// } full_msg_t;


// full_msg_t *notification_get(void);
// void notification_packet_push(uint8_t *data, full_msg_t **message);
void protocol_process_legacy2_notification(const pbl_transport_packet *packet);
