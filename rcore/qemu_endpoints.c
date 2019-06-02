#include <stdbool.h>
#include "qemu.h"
#include "log.h"
#include "protocol.h"

void test_handler(const pbl_transport_packet *packet)
{
    KERN_LOG("QEMU", APP_LOG_LEVEL_INFO, "I got these %d bytes from qemu:", packet->length);
    debug_write((const unsigned char *)"\n", 1);
}

void spp_handler(pbl_transport_packet *packet)
{
    if (!protocol_parse_packet(packet, qemu_send_data))
        return; // we are done, no point looking as we have no data left

    // seems legit
    protocol_process_packet(packet);
}

const PebbleEndpoint qemu_endpoints[] =
{
    {
        .endpoint = QemuProtocol_SPP,
        .handler = spp_handler
    },
    {
        .endpoint = QemuProtocol_Tests,
        .handler = test_handler
    },
    {
        .endpoint = QemuProtocol_BluetoothConnection,
        .handler = test_handler
    },
    { .handler = NULL }
};
