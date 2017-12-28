#pragma once
/* platform_config.h
 * Configuration file for Chalk
 * RebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>
 *          Carson Katri <me@carsonkatri.com>
 */
#include "platform_config_common.h"

#define DISPLAY_ROWS 180
#define DISPLAY_COLS 180

extern unsigned char _binary_Resources_chalk_fpga_bin_size;
extern unsigned char _binary_Resources_chalk_fpga_bin_start;
#define DISPLAY_FPGA_ADDR &_binary_Resources_chalk_fpga_bin_start
#define DISPLAY_FPGA_SIZE &_binary_Resources_chalk_fpga_bin_size

