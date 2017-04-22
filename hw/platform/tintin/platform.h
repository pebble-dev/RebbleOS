#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "tintin.h"
#include "stm32_buttons.h"

#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

#define REGION_PRF_START    0x200000
#define REGION_PRF_SIZE     0x1000000
// DO NOT WRITE TO THIS REGION
#define REGION_MFG_START    0xE0000
#define REGION_MFG_SIZE     0x20000
// Resource start
#define REGION_RES_START    0x280000
#define REGION_RES_SIZE     0x7D000

#define REGION_APP_RES_START    0xB3A000
#define REGION_APP_RES_SIZE     0x7D000

// app slots
#define APP_SLOT_1_START    0xB3E000
#define APP_SLOT_9_START    0xc34000
#define APP_SLOT_17_START    0xc82000
#define APP_HEADER_BIN_OFFSET 0x59
#define APP_SLOT_SIZE       0x48000

#define RES_START           0x100C

#endif
