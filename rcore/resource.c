/* resource.c
 * routines for loading and manipulating resouce files/data
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"


/* Configure Logging */
#define MODULE_NAME "res"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_ERROR



/* The header  is on the flash. it contains the logical location of the
 * resource itself
 */
typedef struct ResHandleFileHeader
{
    uint32_t index;
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
} ResHandleFileHeader;


uint8_t resource_init()
{
    return 0;
}

/* We pass around a pointer to the block of flash or memory where the resource lives */
ResHandleFileHeader _resource_get_res_handle_header(ResHandle res_handle)
{
    ResHandleFileHeader new_header;
    struct fd fd;
    uint8_t is_system = res_handle > REGION_RES_START + RES_TABLE_START &&
                        res_handle < REGION_RES_START + RES_TABLE_START + ((254) * sizeof(ResHandleFileHeader));

    if (is_system)
    {
        flash_read_bytes(res_handle, (uint8_t *)&new_header, sizeof(ResHandleFileHeader));
    }
    else
    {
        App *app = appmanager_get_current_app();
        assert(app && "No App?");
        fs_open(&fd, &app->resource_file);
        fs_seek(&fd, res_handle, FS_SEEK_SET);
        /* get the resource from the flash.
         * each resource is in a big array in the flash, so we get the offsets for the resouce
         * by multiplying out by the size of each resource */
        fs_read(&fd, &new_header, sizeof(ResHandleFileHeader));
    }

//    LOG_DEBUG("Resource sys:%d idx:%d adr:0x%x sz:%d", is_system, new_header.index, new_header.offset, new_header.size);

    // sanity check the resource
    if (new_header.size > 200000) // arbitary 200k
    {
        LOG_ERROR("Res: WARN. Suspect res size %d", new_header.size);
    }

    return new_header;
}


bool _resource_is_sane(ResHandleFileHeader *res_handle)
{
    size_t sz = res_handle->size;

    if (sz <= 0)
    {
        LOG_ERROR("Res: res<=0");
        return false;
    }

    if (sz > 100000)
    {
        LOG_ERROR("Res: malloc will fail. > 100Kb requested");
        return false;
    }

    return true;
}

void _resource_load_file(ResHandleFileHeader resource_header, uint8_t *buffer, size_t max_length, const struct file *file)
{
//    LOG_DEBUG("Loading Start adr:0x%x", resource_header.offset);

    if(!buffer)
    {
        LOG_ERROR("Invalid Buffer");
        return;
    }

    if (!file)
    {
        flash_read_bytes(REGION_RES_START + RES_START + resource_header.offset, buffer, resource_header.size);
        return;
    }

    struct fd fd;
    fs_open(&fd, file);
    fs_seek(&fd, APP_RES_START + resource_header.offset + 0xC, FS_SEEK_SET);
    fs_read(&fd, buffer, max_length ? max_length : resource_header.size);
    return;
}

/*
 * Load a resource fully into a returned buffer
 * By resource handle
 * 
 */
uint8_t *resource_fully_load_resource(ResHandle res_handle, const struct file *file, size_t *loaded_size)
{
    ResHandleFileHeader _handle;
    _handle = _resource_get_res_handle_header(res_handle);

    if (!_resource_is_sane(&_handle))
        return NULL;

    size_t sz = _handle.size;

    uint8_t *buffer = app_calloc(1, sz);
    if (loaded_size)
        *loaded_size = sz;

    if (buffer == NULL)
    {
        LOG_ERROR("Resource alloc of %d bytes for res %d failed", sz, _handle.index);
        return NULL;
    }

    _resource_load_file(_handle, buffer, 0, file);

    return buffer;
}

uint8_t *resource_fully_load_id_system(uint32_t resource_id)
{
    ResHandle res_handle = resource_get_handle_system(resource_id);
    ResHandleFileHeader _handle = _resource_get_res_handle_header(res_handle);
    uint8_t *buffer = resource_fully_load_resource(res_handle, NULL, NULL);

    return buffer;
}

uint8_t *resource_fully_load_id_app(uint32_t resource_id)
{
    ResHandle res_handle = resource_get_handle(resource_id);
    App *app = appmanager_get_current_app();
    uint8_t *buffer = resource_fully_load_resource(res_handle, &app->resource_file, NULL);

    return buffer;
}

uint8_t *resource_fully_load_id_app_file(uint32_t resource_id, const struct file *file, size_t *loaded_size)
{
    ResHandle res_handle = resource_get_handle(resource_id);
    uint8_t *buffer = resource_fully_load_resource(res_handle, file, loaded_size);

    return buffer;
}


/*
 * Exposed public interface
 * 
 */

/*
 * Load up a handle for the resource by ID
 */
ResHandle resource_get_handle_system(uint16_t resource_id)
{
    return REGION_RES_START + RES_TABLE_START + ((resource_id - 1) * sizeof(ResHandleFileHeader));
}

/*
 * Load up a handle for the resource by ID
 */
ResHandle resource_get_handle(uint32_t resource_id)
{
    return ((resource_id - 1) * sizeof(ResHandleFileHeader)) + 0xC;
}

void resource_load(ResHandle resource_handle, uint8_t *buffer, size_t max_length)
{
    App *app = appmanager_get_current_app();
    ResHandleFileHeader _handle = _resource_get_res_handle_header(resource_handle);

    _resource_load_file(_handle, buffer, max_length, &app->resource_file);
}

size_t resource_load_byte_range(ResHandle res_handle, uint32_t start_offset, uint8_t *buffer, size_t num_bytes)
{
    ResHandleFileHeader _handle = _resource_get_res_handle_header(res_handle);

    if (buffer == NULL)
    {
        LOG_ERROR("Invalid buffer");
        return 0;
    }

    if (!_resource_is_sane(&_handle))
        return 0;

    size_t sz = _handle.size;

    App *app = appmanager_get_current_app();

    struct fd fd;
    fs_open(&fd, &app->resource_file);
    fs_seek(&fd, APP_RES_START + _handle.offset + 0xC + start_offset, FS_SEEK_SET);
    fs_read(&fd, buffer, num_bytes);

    return num_bytes;
}

/*
 * return the size of a resource
 */
size_t resource_size(ResHandle handle)
{
    ResHandleFileHeader _handle = _resource_get_res_handle_header(handle);
    return _handle.size;
}

/* XXX MOVE Some missing functionality */

void p_n_grect_standardize(n_GRect r)
{
    n_grect_standardize(r);
}

uintptr_t vApplicationStartSyscall(uint16_t syscall_index)
{
    return 0;
}

GBitmap *gbitmap_create_with_resource_proxy(uint32_t resource_id)
{
    App *app = appmanager_get_current_app();
    return gbitmap_create_with_resource_app(resource_id, &app->resource_file);
}

