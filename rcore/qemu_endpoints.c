#include <stdbool.h>
#include "qemu.h"
#include "log.h"
#include "protocol.h"
#include "test.h"
#include "protocol_service.h"
#include "rebble_memory.h"

void other_handler(const RebblePacket incoming_packet)
{
//     KERN_LOG("QEMU", APP_LOG_LEVEL_INFO, "I got these %d bytes from qemu:", packet_get_data_length(incoming_packet));
//     debug_write((const unsigned char *)"\n", 1);
}

void spp_handler(const RebblePacket incoming_packet)
{
    RebblePacketDataHeader packet;
    
    if (!protocol_parse_packet(packet_get_data(incoming_packet), &packet, qemu_send_data))
        return; // we are done, no point looking as we have no data left

    /* seems legit. We have a valid packet. Create a data packet and process it */
    RebblePacket newpacket = packet_create_with_data(packet.endpoint, packet.data, packet.length);
    
    if (newpacket == NULL)
    {
        KERN_LOG("QEMU", APP_LOG_LEVEL_ERROR, "No Packet memory! Bailing");
        return;
    }
    
//     uint8_t *newdata = packet_get_data(newpacket);
//     memcpy(newdata, packet.data, packet.length);
    
    protocol_process_packet(newpacket);
 
    /* Remove the processed packet from the buffer */
    protocol_rx_buffer_consume(packet.length + sizeof(RebblePacketHeader) + sizeof(QemuCommChannelHeader) + sizeof(QemuCommChannelFooter));
}

const PebbleEndpoint qemu_endpoints[] =
{
    {
        .endpoint = QemuProtocol_SPP,
        .handler = spp_handler
    },
#ifdef REBBLEOS_TESTING
    {
        .endpoint = QemuProtocol_Tests,
        .handler = test_packet_handler
    },
#endif
    {
        .endpoint = QemuProtocol_BluetoothConnection,
        .handler = other_handler
    },
    { .handler = NULL }
};
