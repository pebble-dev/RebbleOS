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
/* Configure Logging */
#define MODULE_NAME "proto"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static void _thread_protocol_tx();

struct rebble_packet {
    void *packet;
    size_t length;
    Uuid *uuid;
    uint32_t endpoint;
    bool needs_ack;
    uint8_t retries;
    uint16_t timeout_ms;
    TickType_t next_time;
//    completion_callback callback;
    list_node node;
};

list_head _messages_awaiting_reponse_head = LIST_HEAD(_messages_awaiting_reponse_head);

#define STACK_SIZE_PROTOCOL_TX (configMINIMAL_STACK_SIZE + 600)
static TaskHandle_t _task_protocol_tx = 0;
static StaticTask_t _task_protocol_tx_tcb;
static StackType_t  _task_protocol_tx_stack[STACK_SIZE_PROTOCOL_TX];

#define PROTOCOL_TX_QUEUE_SIZE 3
static QueueHandle_t         _queue_protocol_tx = 0;
static StaticQueue_t         _queue_protocol_tx_qcb;
static StackType_t _queue_protocol_tx_stack[PROTOCOL_TX_QUEUE_SIZE * sizeof(rebble_packet)];

#define PROTOCOL_MEM_SIZE 2000
static qarena_t *_protocol_arena;
static uint8_t _protocol_heap[PROTOCOL_MEM_SIZE];

void rebble_protocol_init()
{
    _protocol_arena = qinit(_protocol_heap, PROTOCOL_MEM_SIZE);
    
    _queue_protocol_tx = xQueueCreateStatic(PROTOCOL_TX_QUEUE_SIZE, sizeof(rebble_packet), 
                                            (void *)_queue_protocol_tx_stack, &_queue_protocol_tx_qcb);

    _task_protocol_tx = xTaskCreateStatic(_thread_protocol_tx, "Protocol mgr", STACK_SIZE_PROTOCOL_TX, 
                                          NULL, tskIDLE_PRIORITY + 4UL, 
                                          (void *)_task_protocol_tx_stack, &_task_protocol_tx_tcb);
}

void rebble_protocol_send(uint32_t endpoint, Uuid *uuid, void *data, size_t length, 
                          uint8_t retries, uint16_t timeout_ms, 
                          bool needs_ack) /*, callback callback)*/
{
    rebble_packet packet = {
        .packet = data,
        .length = length,
        .uuid = uuid,
        .endpoint = endpoint,
        .needs_ack = needs_ack,
        .retries = retries,
        .timeout_ms = timeout_ms,
        //.callback = callback
    };
    xQueueSendToBack(_queue_protocol_tx, &packet, portMAX_DELAY);
}

void _add_packet_to_send_list(rebble_packet *packet)
{
    // lock
    rebble_packet *pkt_copy = protocol_calloc(1, sizeof(rebble_packet));
    assert(pkt_copy);
    memcpy(pkt_copy, packet, sizeof(rebble_packet));
    TickType_t curtime = xTaskGetTickCount();
    pkt_copy->next_time = curtime + pdMS_TO_TICKS(pkt_copy->timeout_ms);

    list_init_node(&pkt_copy->node);
    list_insert_tail(&_messages_awaiting_reponse_head, &pkt_copy->node);
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
    protocol_free(packet->packet);
    // unlock
}

void rebble_protocol_resend_packet(rebble_packet *packet)
{
    assert(packet);
    // lock    
    TickType_t curtime = xTaskGetTickCount();
    packet->retries--;
    packet->next_time = curtime + pdMS_TO_TICKS(packet->timeout_ms);
    protocol_send_packet(packet->packet); /* send a transport packet */
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
                pbl_transport_packet tpkt = {
                    .length = pkt->length,
                    .endpoint = pkt->endpoint,
                    .data = pkt->packet,
                    .transport_sender = protocol_get_current_transport_sender()
                };
                LOG_ERROR("Timed out sending. Retreies %d", pkt->retries);
                protocol_send_packet(&tpkt); /* send a transport packet */
            }
            else
            {
                /* get the previous element. we are deleting the current one, so we need to repoint */
                rebble_packet *pc = list_elem(list_get_prev(&_messages_awaiting_reponse_head, &pkt->node), rebble_packet, node);
                if (!pc)
                    continue;
                LOG_ERROR("Gave Up");
                rebble_protocol_remove_packet(pkt);
                protocol_free(pkt);
                pkt = pc;
            }
            
        }
    }
}

/*
static void _thread_protocol_rx()
{
    while(1)
    {
        queuereceive(packet);
        parsepacket();
    }
}
*/

static void _thread_protocol_tx()
{
    rebble_packet pkt;
    while(1)
    {
        if(xQueueReceive(_queue_protocol_tx, &pkt, pdMS_TO_TICKS(200)))
        {
            if (pkt.needs_ack)
                _add_packet_to_send_list(&pkt);
            
            pbl_transport_packet tpkt = {
                    .length = pkt.length,
                    .endpoint = pkt.endpoint,
                    .data = pkt.packet,
                    .transport_sender = protocol_get_current_transport_sender()
                };
            protocol_send_packet(&tpkt); /* send a transport packet */
            
            if (!pkt.needs_ack)
                protocol_free(pkt.packet);
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
