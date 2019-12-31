#include <stdbool.h>
#include "qemu.h"
#include "log.h"
#include "protocol.h"
#include "test.h"

void test_handler(const RebblePacketDataHeader *packet)
{
    KERN_LOG("QEMU", APP_LOG_LEVEL_INFO, "I got these %d bytes from qemu:", packet_get_data_length(packet));
    debug_write((const unsigned char *)"\n", 1);
}

void spp_handler(const RebblePacketDataHeader *packet)
{
    if (!protocol_parse_packet(packet, qemu_send_data))
        return; // we are done, no point looking as we have no data left

    /* seems legit. We have a valid packet. Create a data packet and process it */
    RebblePacket newpacket = packet_create(packet->endpoint, packet->length);
    assert(newpacket);
    uint8_t *data = packet_get_data(newpacket);
    memcpy(data, packet->data, packet->length);
    protocol_process_packet(newpacket);
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
