#pragma once
#include "platform.h"

#define QEMU_HEADER_SIGNATURE 0xFEED
#define QEMU_FOOTER_SIGNATURE 0xBEEF
#define QEMU_MAX_DATA_LEN 2048

typedef struct
{
    uint16_t signature; // QEMU_HEADER_SIGNATURE
    uint16_t protocol;  // one of QemuProtocol
    uint16_t len;       // number of bytes that follow (not including this header or footer)
} __attribute__((__packed__)) QemuCommChannelHeader;

typedef struct
{
    uint16_t signature; // QEMU_FOOTER_SIGNATURE
} __attribute__((__packed__)) QemuCommChannelFooter;

enum
{
    // Pebble standard
    QemuProtocol_SPP = 1,
    QemuProtocol_Tap = 2,
    QemuProtocol_BluetoothConnection = 3,
    QemuProtocol_Compass = 4,
    QemuProtocol_Battery = 5,
    QemuProtocol_Accel = 6,
    QemuProtocol_Vibration = 7,
    QemuProtocol_Button = 8,

    // Rebble custom
    QemuProtocol_Tests = 100
};


uint8_t qemu_init(void);
void qemu_reply_test(uint8_t *data, uint16_t len);
void qemu_send_data(uint16_t endpoint, uint8_t *data, uint16_t len);
