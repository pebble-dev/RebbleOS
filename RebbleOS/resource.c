/* resource.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
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




/*
 * 
 * 
 * APP RESOURCES
 * 
 */

/*
 * List all resources for an app
 */
void app_resource_list(char *buffer, uint16_t slot_id)
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
}




uintptr_t vApplicationStartSyscall(uint16_t syscall_index)
{
    printf("APP SYSCALL %d\n");
    return 0;
}
