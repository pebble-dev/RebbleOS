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
#include "qemu.h"

/* Configure Logging */
#define MODULE_NAME "pcolsvc"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_NONE

static void _thread_protocol_rx();
static void _thread_protocol_tx();

struct rebble_packet {
    uint8_t *data;
    size_t length;
    uint32_t endpoint;
    ProtocolTransportSender transport_sender;
//    completion_callback callback;
};

enum {
    ProtocolServiceMessageRX = 0,
    //ProtocolServiceMessageTX = 1,
    ProtocolServiceMessageBufferReset = 2,
};

static CoreTimer *_protocol_timer_head = NULL;

QUEUE_DEFINE(tx, rebble_packet *, 3);
QUEUE_DEFINE(rx, AppMessage, 4);
THREAD_DEFINE(tx, 300, tskIDLE_PRIORITY + 9UL, _thread_protocol_tx);
THREAD_DEFINE(rx, 650, tskIDLE_PRIORITY + 4UL, _thread_protocol_rx);

void rebble_protocol_init()
{
    QUEUE_CREATE(tx);
    QUEUE_CREATE(rx);
    THREAD_CREATE(tx);
    THREAD_CREATE(rx);

    protocol_init();
}

static void _thread_protocol_rx()
{
    AppMessage am;
    RebblePacket newpacket;

    while(1) {

        TickType_t next_timer = appmanager_timer_get_next_expiry(_protocol_timer_head);

        if(next_timer == 0) {
            appmanager_timer_expired(&_protocol_timer_head, _protocol_timer_head);
            next_timer = appmanager_timer_get_next_expiry(_protocol_timer_head);
        }
        if (next_timer < 0)
            next_timer = portMAX_DELAY;
        
        if (!xQueueReceive(QUEUE_HANDLE(rx), &am, next_timer))
            continue;
        
        /* XXX TODO
        if (am.command == ProtocolServiceMessageBufferReset) {

        }*/
        
        if (am.command != ProtocolServiceMessageRX)
            continue;
        
        RebblePacket packet = am.data;

        LOG_DEBUG("packet thread woke");
        
        ProtocolTransportSender transport = packet->transport_sender;
        packet_destroy(packet);

        /* Go around the buffer until we have no more */
        while(1) {
            LOG_DEBUG("processing packet");
            RebblePacketDataHeader hdr;

            int rv = protocol_parse_packet(protocol_get_rx_buffer(), &hdr, transport);
            
            if (rv == PACKET_MORE_DATA_REQD) {
                break;
            }
            else if (rv == PACKET_INVALID) {
                LOG_ERROR("invalid packet. buffer discarded");
                protocol_rx_buffer_reset();
                break;
            }

            if (protocol_buffer_lock() < 0)
                break;
            
            /* seems legit. We have a valid packet. Create a data packet and process it */
            newpacket = packet_create_with_data(hdr.endpoint, hdr.data, hdr.length);
            packet_set_transport(newpacket, transport);
            
            LOG_DEBUG("packet valid RX %x %d", newpacket->transport_sender, hdr.length);
            
            EndpointHandler handler = protocol_find_endpoint_handler(packet_get_endpoint(newpacket), protocol_get_pebble_endpoints());
            if (handler == NULL) {
                LOG_ERROR("unknown endpoint %d", newpacket->endpoint);
            }
            else {
                handler(newpacket);
            }
            
            protocol_buffer_unlock();
            /* Remove the data we processed from the buffer */
            protocol_rx_buffer_consume(newpacket->length + sizeof(RebblePacketHeader));

            packet_destroy(newpacket);
        }
    }
}

static void _packet_tx(const RebblePacket packet)
{
    if (!packet->transport_sender)
        packet->transport_sender = protocol_get_current_transport_sender();

    protocol_send_packet(packet); /* send a transport packet */
    packet_destroy(packet);
}

static void _thread_protocol_tx()
{
    rebble_packet *packet;
    while(1) {
        xQueueReceive(QUEUE_HANDLE(tx), &packet, portMAX_DELAY);
        if (!packet)
            continue;
            
        if (protocol_transaction_lock(10) < 0) {
            xQueueSendToFront(QUEUE_HANDLE(tx), packet, 0); /* requeue */
        }
        else {
            _packet_tx(packet);
            protocol_transaction_unlock();
        }
    }
}

RebblePacket packet_create(uint16_t endpoint, uint16_t size)
{
    void *data = calloc(1, sizeof(rebble_packet) + size);
    if (!data)
        return NULL;
    rebble_packet *rp = data;
    rp->data = (void *)(rp + 1);
    rp->length = size;
    rp->endpoint = endpoint;
    return rp;
}

RebblePacket packet_create_with_data(uint16_t endpoint, uint8_t *data, uint16_t length)
{
    rebble_packet *rp = calloc(1, sizeof(rebble_packet));
    
    rp->data = data;
    rp->length = length;
    rp->endpoint = endpoint;
    return rp;
}

void packet_destroy(RebblePacket packet)
{
    remote_free(packet);
}

int packet_send(const RebblePacket packet)
{
    if (xTaskGetCurrentTaskHandle() == THREAD_HANDLE(rx)) {
        _packet_tx(packet);
        return 0;
    }

    xQueueSendToBack(QUEUE_HANDLE(tx), &packet, portMAX_DELAY);

    return 0;
}

int packet_reply(RebblePacket packet, uint8_t *data, uint16_t size)
{
    RebblePacket newpacket = packet_create(packet->endpoint, size);    
    packet_copy_data(newpacket, data, size);
    newpacket->transport_sender = packet->transport_sender;
    packet_send(newpacket);
    return 0;
}

int packet_recv(const RebblePacket packet)
{
    AppMessage am = {
        .command = ProtocolServiceMessageRX,
        .data = packet
    };
    if (xQueueSendToBack(QUEUE_HANDLE(rx), &am, portMAX_DELAY))
        return 0;
    
    return -1;
}

inline uint8_t *packet_get_data(RebblePacket packet)
{
    return packet->data;
}

inline void packet_set_data(RebblePacket packet, void *data)
{
    packet->data = data;
}

inline void packet_copy_data(RebblePacket packet, void *data, uint16_t size)
{
    memcpy(packet->data, data, size);
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

inline ProtocolTransportSender packet_get_transport(RebblePacket packet)
{
    return packet->transport_sender;
}

inline void packet_set_transport(RebblePacket packet, ProtocolTransportSender transport)
{
    packet->transport_sender = transport;
}


void packet_send_to_transport(RebblePacket packet, uint16_t endpoint, uint8_t *data, uint16_t len)
{
    rebble_packet *pkt = (rebble_packet *)packet;
    pkt->transport_sender(endpoint, data, len);
}


/* Timer */

ProtocolTimer *protocol_service_timer_create(ProtocolTimerCallback pcallback, TickType_t timeout)
{
    ProtocolTimer *ct = mem_heap_alloc(&mem_heaps[HEAP_LOWPRIO], sizeof(ProtocolTimer));
    assert(ct);
    memset(ct, 0, sizeof(ProtocolTimer));

    ct->timer.callback = pcallback;
    ct->timeout_ms = timeout;
    return ct;
}

void protocol_service_timer_destroy(ProtocolTimer *timer)
{
    if (!timer)
        return;

    if (timer->on_queue)
        appmanager_timer_remove(&_protocol_timer_head, (CoreTimer *)timer);
    remote_free(timer);
}

void protocol_service_timer_cancel(ProtocolTimer *timer)
{
    if (timer->on_queue)
        appmanager_timer_remove(&_protocol_timer_head, (CoreTimer *)timer);
    timer->on_queue = 0;
}

void protocol_service_timer_start(ProtocolTimer *timer, TickType_t timeout)
{
    assert(timer);
    if (timer->on_queue)
        return;
    appmanager_timer_add(&_protocol_timer_head, (CoreTimer *)timer);
    timer->on_queue = 1;
    timer->timeout_ms = timeout;
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(timer->timeout_ms);
}

void protocol_service_timer_restart(ProtocolTimer *timer)
{
    assert(timer);
    if (timer->on_queue) {
        appmanager_timer_remove(&_protocol_timer_head, (CoreTimer *)timer);
        timer->on_queue = 0;
    }
    timer->timer.when = xTaskGetTickCount() + pdMS_TO_TICKS(timer->timeout_ms);
    appmanager_timer_add(&_protocol_timer_head, (CoreTimer *)timer);
    timer->on_queue = 1;
}
