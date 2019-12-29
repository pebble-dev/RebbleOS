/* protocol_call.c
 * R/Pebble Protocol Phone and Call requests.
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol.h"
#include "pebble_protocol.h"
#include "protocol_service.h"

/* Configure Logging */
#define MODULE_NAME "p-call"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE





uint8_t pascal_string_to_string(uint8_t *pstr, uint8_t *result_buf)
{
    uint8_t len = (uint8_t)pstr[0];
    /* Byte by byte copy the src to the dest */
    for(int i = 0; i < len; i++)
        result_buf[i] = pstr[i+1];
    
    /* and null term it */
    result_buf[len] = 0;
    
    return len + 1;
}

void protocol_phone_message_process(const pbl_transport_packet *packet)
{
    phone_message *msg = (phone_message *)packet->data;
    rebble_phone_message pmsg;
    memcpy(&pmsg, msg, sizeof(phone_message));
    LOG_DEBUG("Message l %d", packet->length);
    
    if (packet->length > sizeof(phone_message))
    {
        /* We have extended attributes */
        int len = 0;
        /* Convert the strings from pascal strings to normal */
        len = pascal_string_to_string(msg->pascal_string_data, msg->pascal_string_data);
        pmsg.number = msg->pascal_string_data;
        pmsg.name   = msg->pascal_string_data + len;
        len = pascal_string_to_string(msg->pascal_string_data + len, msg->pascal_string_data + len);
    }
    
    notification_show_incoming_call(&pmsg);
    return;
    
    
    
    
    
    switch(msg->command_id)
    {
        case PhoneMessage_IncomingCall:
            LOG_INFO("Incoming Call %s, %s", pmsg.number, pmsg.name);
            notification_show_incoming_call(&pmsg);
            break;
        case PhoneMessage_MissedCall:
            LOG_INFO("Missed Call %s, %s", pmsg.number, pmsg.name);
            break;
        case PhoneMessage_CallStart:
            LOG_INFO("Call Started %s, %s", pmsg.number, pmsg.name);
            break;
        case PhoneMessage_CallEnd:
            LOG_INFO("Call End %s, %s", pmsg.number, pmsg.name);
            break;
        case PhoneMessage_Ring:
            LOG_INFO("Call Ring %s, %s", pmsg.number, pmsg.name);
            break;
        case PhoneMessage_PhoneStateResponse:
            LOG_INFO("Phone State");
            // payload is a pascal list of status packets. each list entry prefixed by length byte
            break;
        default:
            LOG_ERROR("Unknown Command %d", msg->command_id);    
    }
}

void protocol_phone_answer()
{
    protocol_phone_message_send(PhoneMessage_AnswerCall, 0, false);
}

void protocol_phone_hangup()
{
    protocol_phone_message_send(PhoneMessage_HangupCall, 0, false);
}

void protocol_phone_get_state()
{
    protocol_phone_message_send(PhoneMessage_PhoneStateRequest, 0, true);
}

void protocol_phone_message_send(uint8_t command_id, uint32_t cookie, bool needs_ack)
{
    if (!cookie)
    {
        int tc = xTaskGetTickCount();
        cookie = (uint32_t)tc;
    }
    
    phone_message *pm = protocol_calloc(1, sizeof(phone_message));
    pm->command_id = command_id;
    pm->cookie = cookie;
    
    /* Send a phone action */
    Uuid uuid;
    memcpy(&uuid, &cookie, 4);
    rebble_protocol_send(WatchProtocol_PhoneMessage, &uuid, pm, sizeof(phone_message), 
                          3, 1500, needs_ack);
}

