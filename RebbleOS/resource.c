/* resource.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"


uint32_t _resource_get_app_res_slot_address(uint16_t slot_id);

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
 * Load up a handle for the resource by ID
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
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: WARN. Suspect res id %d size %d", resource_id, resHandle.size);
        while(1);
    }

    return resHandle;
}

/*
 * Load up a handle for the resource by ID
 */
ResHandle resource_get_handle_app(uint32_t resource_id, uint16_t slot_id)
{
    ResHandle resHandle;

     if (resource_id == 0)
//          return resHandle;
        while(1);
    uint32_t res_base = _resource_get_app_res_slot_address(slot_id) +  ((resource_id - 1) * sizeof(ResHandle));
    
    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Resource base %x %d", res_base, resource_id);
    
    
    // get the resource from the flash.
    // each resource is in a big array in the flash, so we get the offsets for the resouce
    // by multiplying out by the size of each resource
    flash_read_bytes(res_base, &resHandle, sizeof(ResHandle));
    
    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Resource %d %x %x", resHandle.index, resHandle.offset, resHandle.size);

    // sanity check the resource
    if (resHandle.size > 200000) // arbitary 200k
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: WARN. Suspect res size %d", resHandle.size);
        while(1);
    }

    return resHandle;
}

void resource_load_app(ResHandle resource_handle, uint8_t *buffer, uint16_t slot_id)
{
    if (resource_handle.size > xPortGetFreeAppHeapSize())
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: malloc fail. Not enough heap for %d", resource_handle.size);
        return NULL;
    }
    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Res: Start %p", _resource_get_app_res_slot_address(slot_id) + APP_RES_START + resource_handle.offset);
    uint16_t ofs = 0;
    if (resource_handle.index > 1)
        ofs = 0x1C;
    flash_read_bytes(_resource_get_app_res_slot_address(slot_id) + APP_RES_START + resource_handle.offset + ofs, buffer, resource_handle.size);
}


uint32_t _resource_get_app_res_slot_address(uint16_t slot_id)
{
    uint32_t res_addr = 0;
    
    if (slot_id < 8)
        res_addr = APP_SLOT_0_START + (slot_id * APP_SLOT_SIZE);
    else if (slot_id < 16)
        res_addr =  APP_SLOT_8_START + ((slot_id - 8) * APP_SLOT_SIZE);
    else if (slot_id < 24)
        res_addr = APP_SLOT_16_START + ((slot_id - 16) * APP_SLOT_SIZE);
    else if (slot_id < 32)
        res_addr = APP_SLOT_24_START + ((slot_id - 24) * APP_SLOT_SIZE);
    
    res_addr += APP_HEADER_BIN_OFFSET + RES_TABLE_START - APP_RES_TABLE_OFFSET;
    
    return res_addr;
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
    
    if (resource_handle.size > xPortGetFreeAppHeapSize())
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Ress: malloc fail. Not enough heap for %d", resource_handle.size);
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

uint8_t *resource_fully_load_id_app(uint16_t resource_id, uint16_t slot_id)
{
    ResHandle res = resource_get_handle_app(resource_id, slot_id);
    return resource_fully_load_res_app(res, slot_id);
}


bool _resource_is_sane(ResHandle res_handle)
{
    size_t sz = resource_size(res_handle);
    
    if (sz <= 0)
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: res<=0");
        return false;
    }
    
    if (sz > xPortGetFreeAppHeapSize())
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: malloc fail. Not enough heap for %d", sz);
        return false;
    }
    
    if (sz > 100000)
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: malloc will fail. > 100Kb requested");
        return false;
    }
    
    return true;
}


/*
 * Load a resource fully into a returned buffer
 * By resource handle
 * 
 */
uint8_t *resource_fully_load_res(ResHandle res_handle)
{
    if (!_resource_is_sane(res_handle))
        return NULL;
    
    size_t sz = resource_size(res_handle);
    
    uint8_t *buffer = app_calloc(1, sz);
    
    if (buffer == NULL)
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Resource alloc failed");
        return NULL;
    }

    resource_load(res_handle, buffer);

    return buffer;
}

uint8_t *resource_fully_load_res_app(ResHandle res_handle, uint16_t slot_id)
{
    if (!_resource_is_sane(res_handle))
        return NULL;
    
    size_t sz = resource_size(res_handle);
 
    uint8_t *buffer = app_calloc(1, sz);
    
    if (buffer == NULL)
    {
        KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Resource alloc failed");
        return NULL;
    }
    resource_load_app(res_handle, buffer, slot_id);
    return buffer;
}
    


/*
 * 
 * 
 * APP RESOURCES
 * 
 */

/*
 * List all resources for an app
 */
/*void app_resource_list(char *buffer, uint16_t slot_id)
{
    ResourceHeader res_header;
    ResHandle handle;
    
    
    flash_read_bytes(REGION_APP_RES_START + APP_HEADER_START, &res_header, sizeof(ResourceHeader));
    printf("SIZE: %s %d\n", res_header.resource_name, res_header.resource_list_count);
    
    for (int i = 0; i < res_header.resource_list_count; i++)
    {
        flash_read_bytes(REGION_APP_RES_START + APP_RES_TABLE_START + ((i) * sizeof(ResHandle)), &handle, sizeof(ResHandle));
        
        printf("H i:%d sz:%d of:0x08%x\n", i, handle.size, handle.offset);
    }
}*/




uintptr_t vApplicationStartSyscall(uint16_t syscall_index)
{
//     printf("APP SYSCALL %d\n");
    return 0;
}
