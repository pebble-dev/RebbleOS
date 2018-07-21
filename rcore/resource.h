#pragma once
/* resource.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "graphics_reshandle.h"

struct file;

uint8_t resource_init();
void resource_load_id_system(uint16_t resource_id, uint8_t *buffer);
ResHandle resource_get_handle_system(uint16_t resource_id);
ResHandle resource_get_handle_app(uint32_t resource_id, const struct file *file);
void resource_load_app(ResHandle resource_handle, uint8_t *buffer, const struct file *file);
void resource_load_system(ResHandle resource_handle, uint8_t *buffer);
size_t resource_size(ResHandle handle);
uint8_t *resource_fully_load_id_app(uint16_t resource_id, const struct file *file);
uint8_t *resource_fully_load_id_system(uint16_t resource_id);
uint8_t *resource_fully_load_res_system(ResHandle res_handle);
uint8_t *resource_fully_load_res_app(ResHandle res_handle, const struct file *file);
