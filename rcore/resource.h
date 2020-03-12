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
void resource_load_file(ResHandleFileHeader resource_header_handle, uint8_t *buffer, size_t max_length, const struct file *file);
bool _resource_is_sane(ResHandleFileHeader *res_handle);

uint8_t *resource_fully_load_id_system(uint32_t resource_id);
uint8_t *resource_fully_load_id_app(uint32_t resource_id);
uint8_t *resource_fully_load_id_app_file(uint32_t resource_id, const struct file *file, size_t *loaded_size);
uint8_t *resource_fully_load_resource(ResHandle res_handle, const struct file *file, size_t *loaded_size);
