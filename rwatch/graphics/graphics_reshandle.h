#pragma once
/* graphics_reshandle.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

typedef struct ResHandle
{
    uint32_t index;
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
} ResHandle;

ResHandle resource_get_handle(uint16_t resource_id);
