#pragma once
/* resource.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "graphics_reshandle.h"
#include "fs.h"

typedef struct ResHandleFileHeader ResHandleFileHeader;

uint8_t resource_init();
ResHandle resource_get_handle_system(uint16_t resource_id);
ResHandle resource_get_handle(uint32_t resource_id);
void resource_file_from_file_handle(struct file *file, const struct file *appres, ResHandle hnd);
void resource_file(struct file *file, ResHandle hnd);

size_t resource_size(ResHandle handle);
size_t resource_load_byte_range(ResHandle res_handle, uint32_t start_offset, uint8_t *buffer, size_t num_bytes);
void resource_load(ResHandle resource_handle, uint8_t *buffer, size_t max_length);

uint8_t *resource_fully_load_file(struct file *file, size_t *loaded_size);
