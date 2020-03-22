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
#include "event_service.h"

/* Configure Logging */
#define MODULE_NAME "p-call"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE


static void _protocol_phone_event_destroy(RebblePacket packet)
{
    packet_destroy(packet);
}

void protocol_phone_message_process(const RebblePacket packet)
{   
    event_service_post(EventServiceCommandCall, (void *)packet, (void *)_protocol_phone_event_destroy);
}

rebble_phone_message *protocol_phone_create(uint8_t *data, uint16_t length)
{
    phone_message *msg = (phone_message *)data;
    /* We are allocating on the app's heap here */
    rebble_phone_message *pmsg = app_calloc(1, sizeof(rebble_phone_message));
    memcpy(pmsg, data, sizeof(phone_message));
    
    if (length > sizeof(phone_message))
    {
        /* We have extended attributes */
        int len1 = 0;
        int len2 = 0;
        /* Convert the strings from pascal strings to normal */
        len1 = pascal_string_to_string(msg->pascal_string_data, msg->pascal_string_data);
        pmsg->number = app_calloc(1, len1 + 1);
        memcpy(pmsg->number, msg->pascal_string_data, len1 + 1);
        
        len2 = pascal_string_to_string(msg->pascal_string_data + len1, msg->pascal_string_data + len1);
        pmsg->name = app_calloc(1, len2 + 1);
        memcpy(pmsg->name, msg->pascal_string_data + len1, len2 + 1);
        LOG_DEBUG("Decoded Phone Message: Name %s Num %s Len %d", pmsg->name, pmsg->number, length);
    }
    else
        LOG_DEBUG("Decoded Phone Message: Cmd 0x%02x, Len %d", msg->command_id, length);
    
    return pmsg;
}

void protocol_phone_destroy(rebble_phone_message *msg)
{
    app_free(msg->name);
    app_free(msg->number);
    app_free(msg);
}

/* Client API */

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
    
    phone_message pm = {
        .command_id = command_id,
        .cookie = cookie,
    };
    
    /* Send a phone action */
    Uuid uuid;
    memcpy(&uuid, &cookie, 4);
    RebblePacket packet = packet_create_with_data(WatchProtocol_PhoneMessage, (void *)&pm, sizeof(phone_message));
    
    packet_send(packet);
}

