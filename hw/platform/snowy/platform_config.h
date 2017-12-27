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
#define PBL_RECT

extern unsigned char _binary_Resources_snowy_fpga_bin_size;
extern unsigned char _binary_Resources_snowy_fpga_bin_start;
#define DISPLAY_FPGA_ADDR &_binary_Resources_snowy_fpga_bin_start
#define DISPLAY_FPGA_SIZE &_binary_Resources_snowy_fpga_bin_size
