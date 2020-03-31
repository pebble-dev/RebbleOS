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
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static void _thread_protocol_rx();
static void _thread_protocol_tx();

struct rebble_packet {
    uint8_t *data;
    size_t length;
    uint32_t endpoint;
    ProtocolTransportSender transport_sender;
//    completion_callback callback;
};

QUEUE_DEFINE(tx, rebble_packet *, 3);
QUEUE_DEFINE(rx, rebble_packet *, 3);
THREAD_DEFINE(tx, 300, tskIDLE_PRIORITY + 4UL, _thread_protocol_tx);
THREAD_DEFINE(rx, 300, tskIDLE_PRIORITY + 4UL, _thread_protocol_rx);

void rebble_protocol_init()
{
    QUEUE_CREATE(tx);
    QUEUE_CREATE(rx);
    THREAD_CREATE(tx);
    THREAD_CREATE(rx);
}


static void _thread_protocol_rx()
{
    RebblePacket packet;
    RebblePacket newpacket;

    while(1)
    {
        xQueueReceive(QUEUE_HANDLE(rx), &packet, portMAX_DELAY);
        ProtocolTransportSender transport = packet->transport_sender;
        RebblePacketDataHeader hdr;
        if (protocol_parse_packet(protocol_get_rx_buffer(), &hdr, transport) != PACKET_PROCESSED)
            continue; // Not a valid packet

        /* seems legit. We have a valid packet. Create a data packet and process it */
        newpacket = packet_create_with_data(hdr.endpoint, hdr.data, hdr.length);
        packet_set_transport(newpacket, transport);
        
        LOG_ERROR("PACKET RX %x %x", newpacket->transport_sender, qemu_send_data);
        EndpointHandler handler = protocol_find_endpoint_handler(packet_get_endpoint(newpacket), protocol_get_pebble_endpoints());
        if (handler == NULL)
        {
            LOG_ERROR("unknown endpoint %d", newpacket->endpoint);
            protocol_rx_buffer_consume(newpacket->length + sizeof(RebblePacketHeader));
            continue;
        }

        handler(newpacket);
        
        // consume buffer
        protocol_rx_buffer_consume(&newpacket->length + sizeof(RebblePacketHeader));

        packet_destroy(packet);
        packet_destroy(newpacket);
    }
}

static void _packet_tx(const RebblePacket packet)
{
    if (!packet->transport_sender)
        packet->transport_sender = protocol_get_current_transport_sender();

    LOG_ERROR("PACKET TX %x %x", packet->transport_sender, qemu_send_data);
    protocol_send_packet(packet); /* send a transport packet */
        
    packet_destroy(packet);
}

static void _thread_protocol_tx()
{
    rebble_packet *packet;
    while(1)
    {
        xQueueReceive(QUEUE_HANDLE(tx), &packet, portMAX_DELAY);
        _packet_tx(packet);
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
    xQueueSendToBack(QUEUE_HANDLE(rx), &packet, portMAX_DELAY);
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
