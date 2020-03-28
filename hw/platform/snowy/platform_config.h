#pragma once
/* platform_config.h
 * Configuration file for Snowy
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "platform_config_common.h"

#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

//We are a square device
#ifndef PBL_RECT
#define PBL_RECT
#endif

extern unsigned char _binary_Resources_snowy_fpga_bin_size;
extern unsigned char _binary_Resources_snowy_fpga_bin_start;
#define DISPLAY_FPGA_ADDR &_binary_Resources_snowy_fpga_bin_start
#define DISPLAY_FPGA_SIZE &_binary_Resources_snowy_fpga_bin_size


/* Bluetooth config */
#define BLUETOOTH_MODULE_TYPE        BLUETOOTH_MODULE_TYPE_CC2564B
#define BLUETOOTH_MODULE_NAME_LENGTH 0x0d
#define BLUETOOTH_MODULE_LE_NAME     'P', 'e', 'b', 'b', 'l', 'e', ' ', 'T', 'i', 'm', 'e', 'L', 'E'
#define BLUETOOTH_MODULE_GAP_NAME    "Pebble Time RblOs"

#define MEM_REGION_DISPLAY  MEM_REGION_CCRAM
#define MEM_REGION_RAMFS    MEM_REGION_CCRAM
#define MEM_REGION_HEAP_OVL MEM_REGION_CCRAM
#define MEM_REGION_HEAP_WRK 
#define MEM_REGION_PANIC    MEM_REGION_CCRAM
