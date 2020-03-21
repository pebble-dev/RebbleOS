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

bool _resource_is_sane(ResHandleFileHeader *res_handle)
{
    size_t sz = res_handle->size;
    app_running_thread *_this_thread = appmanager_get_current_thread();
    App *_running_app = _this_thread->app;
    uint32_t total_app_size = &_running_app->header->app_size;
    uint32_t app_heap_size = _this_thread->heap_size - total_app_size;


    if (sz <= 0)
    {
        LOG_ERROR("Res: res<=0");
        return false;
    }

    if (sz > app_heap_size)
    {
        LOG_ERROR("Res: Attempted to load resource that is larger than the remaining memory");
        return false;
    }

    return true;
}



/*
 * Exposed public interface
 * 
 */

ResHandle resource_get_handle_system(uint16_t resource_id)
{
    return REGION_RES_START + RES_TABLE_START + ((resource_id - 1) * sizeof(ResHandleFileHeader));
}

ResHandle resource_get_handle(uint32_t resource_id)
{
    return ((resource_id - 1) * sizeof(ResHandleFileHeader)) + 0xC;
}

void resource_file_from_file_handle(struct file *file, const struct file *appres, ResHandle hnd)
{
    assert(hnd < REGION_RES_START);
    
    struct fd fd;
    ResHandleFileHeader hdr;
    
    fs_open(&fd, appres);
    fs_seek(&fd, hnd, FS_SEEK_SET); /* XXX: error check if we're out of bounds */
    fs_read(&fd, &hdr, sizeof(hdr));
    
    if (!_resource_is_sane(&hdr)) {
        LOG_ERROR("resource_file_from_file_handle: resource is not sane (sz %d); returning nothing", hdr.size);
        memset(file, 0, sizeof(*file));
        return;
    }
    
    fs_file_from_file(file, appres, APP_RES_START + hdr.offset + 0xC, hdr.size);
}

void resource_file(struct file *file, ResHandle hnd) 
{
    if (hnd < REGION_RES_START)
    {
        App *app = appmanager_get_current_app();
        assert(app && "resource_to_file on app resource without running app");
        
        resource_file_from_file_handle(file, &app->resource_file, hnd);
    } else {
        ResHandleFileHeader hdr;
        
        flash_read_bytes(hnd, &hdr, sizeof(hdr));
        
        if (!_resource_is_sane(&hdr)) {
            LOG_ERROR("resource_file: system resource is not sane (!!; sz %d); returning nothing", hdr.size);
            memset(file, 0, sizeof(*file));
            return;
        }
        
        fs_file_from_flash(file, REGION_RES_START + RES_START + hdr.offset, hdr.size);
    }
}

void resource_load(ResHandle hnd, uint8_t *buffer, size_t max_length)
{
    struct file file;
    struct fd fd;
    
    resource_file(&file, hnd);
    fs_open(&fd, &file);
    fs_read(&fd, buffer, max_length);
}

size_t resource_load_byte_range(ResHandle hnd, uint32_t start_offset, uint8_t *buffer, size_t num_bytes)
{
    struct file file;
    struct fd fd;

    resource_file(&file, hnd);
    fs_open(&fd, &file);
    fs_seek(&fd, start_offset, FS_SEEK_SET);
    return fs_read(&fd, buffer, num_bytes);
}

size_t resource_size(ResHandle hnd)
{
    struct file file;
    struct fd fd;

    resource_file(&file, hnd);
    fs_open(&fd, &file);
    return fs_size(&fd);
}

uint8_t *resource_fully_load_file(struct file *file, size_t *loaded_size)
{
    struct fd fd;
    
    fs_open(&fd, file);
    size_t sz = fs_size(&fd);
    
    if (sz == 0) {
        LOG_ERROR("cowardly refusing to load a zero-byte resource", sz);
        if (loaded_size)
            *loaded_size = 0;
        return NULL;
    }
    
    void *buf = app_calloc(1, sz);
    if (!buf) {
        LOG_ERROR("resource alloc of %d bytes failed", sz);
        if (loaded_size)
            *loaded_size = 0;
        return NULL;
    }
    
    size_t rsz = fs_read(&fd, buf, sz);
    assert(rsz == sz);
    
    if (loaded_size)
        *loaded_size = sz;
    
    return buf;
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

