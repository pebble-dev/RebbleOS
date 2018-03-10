/* resource.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"

extern size_t xPortGetFreeAppHeapSize(void);

static SemaphoreHandle_t _res_mutex;
static StaticSemaphore_t _res_mutex_buf;

uint32_t _resource_get_app_res_slot_address(const struct file *file);

void resource_init()
{
    _res_mutex = xSemaphoreCreateMutexStatic(&_res_mutex_buf);
}

/*
 * Load up a resource for the resource ID into the given buffer
 */
void resource_load_id_system(uint16_t resource_id, uint8_t *buffer)
{
    ResHandle handle;
    if (resource_id > 65000) // arbitary
    {
        buffer = NULL;
        return;
    }
    handle = resource_get_handle_system(resource_id);

    flash_read_bytes(REGION_RES_START + RES_START + handle.offset, buffer, handle.size);
}

/*
 * Load up a handle for the resource by ID
 */
ResHandle resource_get_handle_system(uint16_t resource_id)
{
    xSemaphoreTake(_res_mutex, portMAX_DELAY);
    ResHandle resHandle;

    // get the resource from the flash.
    // each resource is in a big array in the flash, so we get the offsets for the resouce
    // by multiplying out by the size of each resource
    flash_read_bytes(REGION_RES_START + RES_TABLE_START + ((resource_id - 1) * sizeof(ResHandle)), (uint8_t *)&resHandle, sizeof(ResHandle));

    // sanity check the resource
    if (resHandle.size > 200000) // arbitary 200k
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: WARN. Suspect res id %d size %d", resource_id, resHandle.size);
        while(1);
    }
    xSemaphoreGive(_res_mutex);

    return resHandle;
}

/*
 * Load up a handle for the resource by ID
 */
ResHandle resource_get_handle_app(uint32_t resource_id, const struct file *file)
{
    xSemaphoreTake(_res_mutex, portMAX_DELAY);


    ResHandle resHandle;

     if (resource_id == 0)
     {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "App asked for resource 0. I don't know what that means");
        return resHandle;
     }

    uint32_t res_base = ((resource_id - 1) * sizeof(ResHandle)) + 0xC;

    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Resource base %x %d", res_base, resource_id);

    struct fd fd;
    fs_open(&fd, file);
    fs_seek(&fd, res_base, FS_SEEK_SET);

    // get the resource from the flash.
    // each resource is in a big array in the flash, so we get the offsets for the resouce
    // by multiplying out by the size of each resource
    fs_read(&fd, &resHandle, sizeof(ResHandle));
    
    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Resource %d %x %x", resHandle.index, resHandle.offset, resHandle.size);

    // sanity check the resource
    if (resHandle.size > 200000) // arbitary 200k
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: WARN. Suspect res size %d", resHandle.size);
        while(1);
    }
    xSemaphoreGive(_res_mutex);

    return resHandle;
}

void resource_load_app(ResHandle resource_handle, uint8_t *buffer, const struct file *file)
{
//     if (resource_handle.size > xPortGetFreeAppHeapSize())
//     {
//         KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: malloc fail. Not enough heap for %d", resource_handle.size);
//         return;
//     }
    
    KERN_LOG("resou", APP_LOG_LEVEL_DEBUG, "Res: Start %p", APP_RES_START + resource_handle.offset);
    
    uint16_t ofs = 0xC;
    
//     if (resource_handle.index > 1)
//         ofs += 0x1C;
//

    struct fd fd;
    fs_open(&fd, file);
    fs_seek(&fd, APP_RES_START + resource_handle.offset + ofs, FS_SEEK_SET);
    fs_read(&fd, buffer, resource_handle.size);
    return;
}

/*
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
}*/


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
void resource_load_system(ResHandle resource_handle, uint8_t *buffer)
{
    
//     if (resource_handle.size > xPortGetFreeAppHeapSize())
//     {
//         KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Ress: malloc fail. Not enough heap for %d", resource_handle.size);
//         return;
//     }
    
    xSemaphoreTake(_res_mutex, portMAX_DELAY);
    
    flash_read_bytes(REGION_RES_START + RES_START + resource_handle.offset, buffer, resource_handle.size);
    
    xSemaphoreGive(_res_mutex);

}

/*
 * Load a resource fully into a returned buffer
 * By resource ID
 * 
 */
uint8_t *resource_fully_load_id_system(uint16_t resource_id)
{
    ResHandle res = resource_get_handle_system(resource_id);
    return resource_fully_load_res_system(res);
}

uint8_t *resource_fully_load_id_app(uint16_t resource_id, const struct file *file)
{
    ResHandle res = resource_get_handle_app(resource_id, file);
    return resource_fully_load_res_app(res, file);
}


bool _resource_is_sane(ResHandle res_handle)
{
    size_t sz = resource_size(res_handle);
    
    if (sz <= 0)
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: res<=0");
        return false;
    }
    
    
    /* XXX TODO qalloc to implement this
    if (sz > xPortGetFreeAppHeapSize())
    {
        KERN_LOG("resou", APP_LOG_LEVEL_ERROR, "Res: malloc fail. Not enough heap for %d", sz);
        return false;
    }*/
    
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
uint8_t *resource_fully_load_res_system(ResHandle res_handle)
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

    resource_load_system(res_handle, buffer);

    return buffer;
}

uint8_t *resource_fully_load_res_app(ResHandle res_handle, const struct file *file)
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
    resource_load_app(res_handle, buffer, file);
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



/*
 * Cheesy proxy to get the apps slot_id
 * When we need any resource from an app, we need a way of knowing
 * which app it was that wanted that resource. We know which app is running, that's the apps slot
 * 
 */
GBitmap *gbitmap_create_with_resource_proxy(uint32_t resource_id)
{
    App *app = appmanager_get_current_app();
    return gbitmap_create_with_resource_app(resource_id, &app->resource_file);
}

ResHandle resource_get_handle(uint16_t resource_id)
{
    App *app = appmanager_get_current_app();
    return resource_get_handle_app(resource_id, &app->resource_file);
}

void resource_load(ResHandle resource_handle, uint8_t *buffer, uint32_t size)
{
    App *app = appmanager_get_current_app();
    /* TODO: respect passed size, should we include file in ResHandle? */
    return resource_load_app(resource_handle, buffer, &app->resource_file);
}

/* app proxies by pointer */
ResHandle *resource_get_handle_proxy(uint16_t resource_id)
{
    App *app = appmanager_get_current_app();
    KERN_LOG("app", APP_LOG_LEVEL_DEBUG, "ResH %d %s", resource_id, app->header->name);

    // push to the heap.
    ResHandle *x = app_malloc(sizeof(ResHandle));
    ResHandle y = resource_get_handle_app(resource_id, &app->resource_file);
    memcpy(x, &y, sizeof(ResHandle));
     
    return x;
}

GFont *fonts_load_custom_font_proxy(ResHandle *handle)
{
    App *app = appmanager_get_current_app();
    return (GFont *)fonts_load_custom_font(handle, &app->resource_file);
}


/* XXX MOVE Some missing functionality */

void p_n_grect_standardize(n_GRect r)
{
    n_grect_standardize(r);
}
