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
    packet_recv(incoming_packet);
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
    {
        .endpoint = QemuProtocol_TestsLoopback,
        .handler = test_packet_loopback_handler
    },
#endif
    {
        .endpoint = QemuProtocol_BluetoothConnection,
        .handler = other_handler
    },
    { .handler = NULL }
};
