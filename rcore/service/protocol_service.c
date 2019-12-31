/* protocol_service.c
 * Sends a fuly fledged packet over the transport
 * deals with retries and ack responses
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol_system.h"
#include "protocol_service.h"
#include "pebble_protocol.h"
#include "timeline.h"
#include "FreeRTOS.h"
#include "rtoswrap.h"
/* Configure Logging */
#define MODULE_NAME "proto"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static void _thread_protocol_rx();
static void _thread_protocol_tx();

struct rebble_packet {
    uint8_t *data;
    size_t length;
    Uuid *uuid;
    uint32_t endpoint;
    bool needs_ack;
    bool own_data; /* user provided own data buffer. we won't free it */
    uint8_t retries;
    uint16_t timeout_ms;
    TickType_t next_time;
    ProtocolTransportSender transport_sender;
//    completion_callback callback;
    list_node node;
};

list_head _messages_awaiting_reponse_head = LIST_HEAD(_messages_awaiting_reponse_head);

QUEUE_DEFINE(tx, rebble_packet *, 3);
THREAD_DEFINE(tx, 300, tskIDLE_PRIORITY + 4UL, _thread_protocol_tx);

QUEUE_DEFINE(rx, rebble_packet *, 2);
THREAD_DEFINE(rx, 300, tskIDLE_PRIORITY + 6UL, _thread_protocol_rx);

#define PROTOCOL_MEM_SIZE 2000
static qarena_t *_protocol_arena;
static uint8_t _protocol_heap[PROTOCOL_MEM_SIZE];

void rebble_protocol_init()
{
    _protocol_arena = qinit(_protocol_heap, PROTOCOL_MEM_SIZE);
       
    QUEUE_CREATE(tx);
    THREAD_CREATE(tx);
    QUEUE_CREATE(rx);
    THREAD_CREATE(rx);
}

void rebble_protocol_send(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    RebblePacket packet = packet_create(endpoint, len);
    memcpy(packet->data, data, len);
    packet_send(packet);
}

void _add_packet_to_send_list(rebble_packet *packet)
{
    // lock
    TickType_t curtime = xTaskGetTickCount();
    packet->next_time = curtime + pdMS_TO_TICKS(packet->timeout_ms);

    list_init_node(&packet->node);
    list_insert_tail(&_messages_awaiting_reponse_head, &packet->node);
    // unlock
}

rebble_packet *rebble_protocol_get_awaiting_by_uuid(Uuid *uuid)
{
    rebble_packet *p;
    list_foreach(p, &_messages_awaiting_reponse_head, rebble_packet, node)
    {
        if (uuid_equal(p->uuid, uuid))
            return p;
    }
    return NULL;
}

void rebble_protocol_remove_packet(rebble_packet *packet)
{
    assert(packet);
    // lock    
    list_remove(&_messages_awaiting_reponse_head, &packet->node);
    protocol_free(packet->data);
    // unlock
}

void rebble_protocol_resend_packet(rebble_packet *packet)
{
    assert(packet);
    // lock    
    TickType_t curtime = xTaskGetTickCount();
    packet->retries--;
    packet->next_time = curtime + pdMS_TO_TICKS(packet->timeout_ms);
    protocol_send_packet(packet); /* send a transport packet */
    // unlock
}

static void _check_for_timed_out_packets()
{
    TickType_t curtime = xTaskGetTickCount();

    rebble_packet *pkt;
    list_foreach(pkt, &_messages_awaiting_reponse_head, rebble_packet, node)
    {
        if (pkt->next_time < curtime)
        {
            if (pkt->retries)
            {
                pkt->retries--;
                pkt->next_time = curtime + pdMS_TO_TICKS(pkt->timeout_ms);
                
                pkt->transport_sender = protocol_get_current_transport_sender();
                protocol_send_packet(pkt); /* send a transport packet */
                LOG_ERROR("Timed out sending. Retries %d", pkt->retries);
            }
            else
            {
                /* get the previous element. we are deleting the current one, so we need to repoint */
                rebble_packet *pc = list_elem(list_get_prev(&_messages_awaiting_reponse_head, &pkt->node), rebble_packet, node);
                if (!pc)
                    continue;
                LOG_ERROR("Gave Up");
                rebble_protocol_remove_packet(pkt);
                packet_destroy(pkt);
                pkt = pc;
            }
            
        }
    }
}

void protocol_service_rx_data(RebblePacket packet)
{
    xQueueSendToBack(QUEUE_HANDLE(rx), &packet, portMAX_DELAY);
}

static void _thread_protocol_rx()
{
    rebble_packet *packet;
    while(1)
    {
        if(xQueueReceive(QUEUE_HANDLE(rx), &packet, pdMS_TO_TICKS(200)))
        {
            LOG_ERROR("PS %x", packet->endpoint);
            EndpointHandler handler = protocol_find_endpoint_handler(packet->endpoint, protocol_get_pebble_endpoints());
            if (handler == NULL)
            {
                LOG_ERROR("Unknown protocol: %d", packet->endpoint);
                return;
            }

            handler(packet);
        }        
    }
}

static void _thread_protocol_tx()
{
    rebble_packet *packet;
    while(1)
    {
        if(xQueueReceive(QUEUE_HANDLE(tx), &packet, pdMS_TO_TICKS(200)))
        {
            if (packet->needs_ack)
                _add_packet_to_send_list(packet);
            
            packet->transport_sender = protocol_get_current_transport_sender();
            protocol_send_packet(packet); /* send a transport packet */
            
            packet_destroy(packet);
        }

        _check_for_timed_out_packets();
    }
}

void *protocol_calloc(uint8_t count, size_t size)
{
    void *x = qalloc(_protocol_arena, count * size);
    if (x == NULL)
    {
        LOG_ERROR("!!! NO MEM!");
        return NULL;
    }

    memset(x, 0, count * size);
    return x;
}

void protocol_free(void *mem)
{
    LOG_DEBUG("P Free 0x%x", mem);
    qfree(_protocol_arena, mem);
}


RebblePacket packet_create(uint16_t endpoint, uint16_t length)
{
    void *data = protocol_calloc(1, length);
    if (!data)
        return NULL;
    return packet_create_with_data(endpoint, data, length);
}

RebblePacket packet_create_with_data(uint16_t endpoint, uint8_t *data, uint16_t length)
{
    rebble_packet *rp = protocol_calloc(1, sizeof(rebble_packet));
    
    rp->data = data;
    rp->length = length;
    rp->endpoint = endpoint;
    rp->own_data = true;
    return rp;
}

void packet_destroy(RebblePacket packet)
{
    protocol_free(packet->data);
    protocol_free(packet);
}

void packet_send(RebblePacket packet)
{
    xQueueSendToBack(QUEUE_HANDLE(tx), &packet, portMAX_DELAY);
}

inline uint8_t *packet_get_data(RebblePacket packet)
{
    return packet->data;
}

void packet_set_data(RebblePacket packet, uint8_t *data)
{
    if (!packet->own_data)
        protocol_free(packet->data);
    packet->data = data;
}

inline void packet_set_retries(RebblePacket packet, uint8_t retries)
{
    packet->retries = retries;
}

inline uint8_t packet_get_retries(RebblePacket packet)
{
    return packet->retries;
}

inline void packet_set_timeout(RebblePacket packet, uint32_t timeout_ms)
{
    packet->timeout_ms = timeout_ms;
}

inline uint32_t packet_get_timeout(RebblePacket packet)
{
    return packet->timeout_ms;
}

inline void packet_set_ack_required(RebblePacket packet, Uuid *uuid)
{
    packet->uuid = uuid;
}

inline uint16_t packet_get_data_length(RebblePacket packet)
{
    return packet->length;
}

inline uint16_t packet_get_endpoint(RebblePacket packet)
{
    return packet->endpoint;
}

inline void packet_set_endpoint(RebblePacket packet, uint16_t endpoint)
{
    packet->endpoint = endpoint;
}

void packet_send_to_transport(RebblePacket packet, uint16_t endpoint, uint8_t *data, uint16_t len)
{
    rebble_packet *pkt = (rebble_packet *)packet;
    pkt->transport_sender(endpoint, data, len);
}
