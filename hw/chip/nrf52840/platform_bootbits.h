#pragma once

#include <stdint.h>

enum bootbits {
    BOOT_SUCCESS = 0x1,
    BOOT_FAILED1 = 0x2,
    BOOT_FAILED2 = 0x4,
    BOOT_FWUPD_REQ = 0x8,
    BOOT_FWUPD_COMPLETE = 0x10,
    BOOT_RECOV_REQ = 0x20,
    BOOT_FORCE_SADWATCH = 0x40,
    BOOT_RECOV_LOADED = 0x80,
};

int hw_bootbits_test(uint32_t bootbit);
void hw_bootbits_clear(uint32_t bootbit);
void hw_bootbits_set(uint32_t bootbit);
