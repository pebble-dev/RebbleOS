/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

// TODO
// mutex
// better resource loading
// DMA/async?
// what about apps/watchface resource loading?
// document

void flash_load_resource(uint16_t resource_id, uint8_t *buffer);
ResHandle flash_get_reshandle(uint16_t resource_id);
void flash_load_resource_length(uint16_t resource_id, uint8_t *buffer, size_t offset, size_t num_bytes);

/// MUTEX
static SemaphoreHandle_t flash_mutex;


void flash_init()
{
    flash_mutex = xSemaphoreCreateMutex();
    hw_flash_init();

//     flash_test(120);
}

inline void flash_load_read_bytes(uint32_t address, uint8_t *buffer, size_t offset, size_t num_bytes)
{
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(address, buffer, num_bytes);
    xSemaphoreGive(flash_mutex);
}

void flash_load_resource_id(uint16_t resource_id, uint8_t *buffer)
{
    ResHandle address;
    address = flash_get_reshandle(resource_id - 1);

    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(REGION_RES_START + RES_START + address.offset, buffer, address.size);
    xSemaphoreGive(flash_mutex);
}

void flash_load_resource_from_handle(ResHandle resource_handle, uint8_t *buffer)
{
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(REGION_RES_START + RES_START + resource_handle.offset, buffer, resource_handle.size);
    xSemaphoreGive(flash_mutex);
}

void flash_load_resource_length(uint16_t resource_id, uint8_t *buffer, size_t offset, size_t num_bytes)
{
    ResHandle address;
    address = flash_get_reshandle(resource_id - 1);
//     printf("FLASH ofs %d,idx %d, size %d, crc %d\n", (int)address.offset, (int)address.index, (int)address.size, (int)address.crc);
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(REGION_RES_START + RES_START + address.offset + offset, buffer, num_bytes);
    xSemaphoreGive(flash_mutex);
}

ResHandle flash_get_reshandle(uint16_t resource_id)
{
    ResHandle resHandle;
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    // go to the flash resource headers table
    // can speed this up by memcpy to the struct
    resHandle.index = hw_flash_read32(REGION_RES_START + RES_TABLE_START + (resource_id * 16));
    resHandle.offset = hw_flash_read32(REGION_RES_START + RES_TABLE_START + (resource_id * 16) + 4);
    resHandle.size = hw_flash_read32(REGION_RES_START + RES_TABLE_START + (resource_id * 16) + 8);
    resHandle.crc = hw_flash_read32(REGION_RES_START + RES_TABLE_START + (resource_id * 16) + 12);
//     printf("FLASH ofs %d,idx %d, size %d, crc %d\n", resHandle.offset, resHandle.index, resHandle.size, resHandle.crc);
    xSemaphoreGive(flash_mutex);
    return resHandle;
}

void flash_test(uint16_t resource_id)
{
    // test
    flash_get_reshandle(resource_id - 1);
    uint8_t *buffer = display_get_buffer();//[24000];  // arbitary super massive for testing
    flash_load_resource_id(resource_id, buffer);
    display_draw();
}

// resources
ResHandle resource_get_handle(uint16_t resource_id)
{
    ResHandle handle;
    handle = flash_get_reshandle(resource_id - 1);
    return handle;
}

size_t resource_size(ResHandle handle)
{
    return handle.size;
}


void resource_load(ResHandle resource_handle, uint8_t *buffer, size_t image_size)
{
    printf("resource load\n");
    xSemaphoreTake(flash_mutex, portMAX_DELAY);
    hw_flash_read_bytes(REGION_RES_START + RES_START + resource_handle.offset, buffer, resource_handle.size);
    xSemaphoreGive(flash_mutex);
}
