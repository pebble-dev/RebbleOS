/* protocol_service.c
 * Sends a fuly fledged packet over the transport
 * deals with retries and ack responses.
 * All RX and TX and send through their own threads. 
 * Each thread has a queue for the incoming/outgoing data. 
 * All data from RX is send through the queue as a RebblePacket, processing
 * taken away from the driver to this protocol handler. The packet is then parsed and sent.
 * See header for more details.
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
#define MODULE_NAME "pcolsvc"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static void _thread_protocol_rx();
static void _thread_protocol_tx();

struct rebble_packet {
    uint8_t *data;
    size_t length;
    Uuid *uuid;
    uint32_t endpoint;
    bool own_data; /* user provided own data buffer. we won't free it */
    ProtocolTransportSender transport_sender;
//    completion_callback callback;
    list_node node;
};

QUEUE_DEFINE(tx, rebble_packet *, 3);
THREAD_DEFINE(tx, 300, tskIDLE_PRIORITY + 4UL, _thread_protocol_tx);

QUEUE_DEFINE(rx, rebble_packet *, 2);
THREAD_DEFINE(rx, 300, tskIDLE_PRIORITY + 6UL, _thread_protocol_rx);

#define PROTOCOL_MEM_SIZE 2200
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
    if (!packet)
    {
        LOG_ERROR("No memory for packet");
        return;
    }
    memcpy(packet->data, data, len);
    packet_send(packet);
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
        xQueueReceive(QUEUE_HANDLE(rx), &packet, portMAX_DELAY);
        
        EndpointHandler handler = protocol_find_endpoint_handler(packet->endpoint, protocol_get_pebble_endpoints());
        if (handler == NULL)
        {
            LOG_ERROR("Unknown protocol: %d", packet->endpoint);
            packet_destroy(packet);
            continue;
        }

        handler(packet);
    }
}

static void _thread_protocol_tx()
{
    rebble_packet *packet;
    while(1)
    {
        xQueueReceive(QUEUE_HANDLE(tx), &packet, portMAX_DELAY);
        
        packet->transport_sender = protocol_get_current_transport_sender();
        protocol_send_packet(packet); /* send a transport packet */
        
        packet_destroy(packet);
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
    if (!packet->own_data)
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
