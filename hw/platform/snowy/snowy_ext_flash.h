#pragma once
/* snowy_ext_flash.h
 * FMC NOR flash implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"

void hw_flash_init(void);
void hw_flash_deinit(void);
void hw_flash_read_bytes(uint32_t address, uint8_t *buffer, size_t length);
void *hw_flash_module_init(hw_driver_handler_t *handler);
