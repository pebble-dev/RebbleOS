#pragma once
/* resource.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

void resource_init();
void resource_load_id(uint16_t resource_id, uint8_t *buffer);
// ResHandle resource_get_handle(uint32_t resource_id);
size_t resource_size(ResHandle handle);
void resource_load(ResHandle resource_handle, uint8_t *buffer);
uint8_t *resource_fully_load_id(uint16_t resource_id);
uint8_t *resource_fully_load_res(ResHandle res_handle);
ResHandle resource_get_handle_app(uint32_t resource_id, uint16_t slot_id);
uint8_t *resource_fully_load_res_app(ResHandle res_handle, uint16_t slot_id);
