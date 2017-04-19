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

void resource_init()
{

}

/*
 * Load up a resource for the resource ID into the given buffer
 */
void resource_load_id(uint16_t resource_id, uint8_t *buffer)
{
    ResHandle handle;
    if (resource_id > 65000) // arbitary
    {
        buffer = NULL;
        return;
    }
    handle = resource_get_handle(resource_id);

    flash_read_bytes(REGION_RES_START + RES_START + handle.offset, buffer, handle.size);
}

/*
 * Load up a handle for the resource by IDD
 */
ResHandle resource_get_handle(uint16_t resource_id)
{
    ResHandle resHandle;

    // get the resource from the flash.
    // each resource is in a big array in the flash, so we get the offsets for the resouce
    // by multiplying out by the size of each resource
    flash_read_bytes(REGION_RES_START + RES_TABLE_START + ((resource_id - 1) * sizeof(ResHandle)), &resHandle, sizeof(ResHandle));

    // sanity check the resource
    if (resHandle.size > 200000) // arbitary 200k
    {
        printf("Res: WARN. Suspect res size %d\n", resHandle.size);
    }

    return resHandle;
}

/*
 * return the size of a resource
 */
size_t resource_size(ResHandle handle)
{
    return handle.size;
}

/*
 * Load a resource into the given buffer
 * By resource handle
 * 
 */
void resource_load(ResHandle resource_handle, uint8_t *buffer)
{
    if (resource_handle.size > xPortGetFreeHeapSize())
    {
        printf("Res: malloc fail. Not enough heap for %d\n", resource_handle.size);
        return NULL;
    }
    flash_read_bytes(REGION_RES_START + RES_START + resource_handle.offset, buffer, resource_handle.size);
}

/*
 * Load a resource fully into a returned buffer
 * By resource ID
 * 
 */
uint8_t *resource_fully_load_id(uint16_t resource_id)
{
    ResHandle res = resource_get_handle(resource_id);
    return resource_fully_load_res(res);
}

/*
 * Load a resource fully into a returned buffer
 * By resource handle
 * 
 */
uint8_t *resource_fully_load_res(ResHandle res_handle)
{
    size_t sz = resource_size(res_handle);

    if (sz > xPortGetFreeHeapSize())
    {
        printf("Res: malloc fail. Not enough heap for %d\n", sz);
        return NULL;
    }
    
    if (sz > 100000)
    {
        printf("Res: malloc will fail. > 100Kb requested\n");
        return NULL;
    }
    uint8_t *buffer = rbl_calloc(1, sz);
    
    if (buffer == NULL)
    {
        printf("font alloc failed\n");
        return NULL;
    }

    resource_load(res_handle, buffer);

    return buffer;
}
