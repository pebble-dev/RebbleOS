/* rebble_memory.c
 * routines for Allocating memory for system and apps. 
 * App memory gets allocated in it's relevant arena heap. System has the rest
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

/* Configure Logging */
#define MODULE_NAME "mem"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_NONE

void rblos_memory_init(void)
{
}

void *system_calloc(size_t count, size_t size)
{
    if (!appmanager_is_thread_system())
        LOG_ERROR("XXX System Calloc. Check who did this");

    void *x = pvPortMalloc(count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

void *system_malloc(size_t size)
{
    if (appmanager_is_thread_system())
        LOG_ERROR("XXX System Malloc. Check who did this");
    return pvPortMalloc(size);
}

void *app_malloc(size_t size)
{
    return app_calloc(1, size);    
}

void *app_calloc(size_t count, size_t size)
{
    app_running_thread *thread = appmanager_get_current_thread();
    assert(thread && "invalid thread");
    void *x = qalloc(thread->arena, count * size);
    if (x == NULL)
    {
        LOG_ERROR("!!! NO MEM!\n");
        return NULL;
    }

    memset(x, 0, count * size);
    return x;
}

void app_free(void *mem)
{
    LOG_DEBUG("Free 0x%x", mem);
    app_running_thread *thread = appmanager_get_current_thread();
    qfree(thread->arena, mem);
}

void *app_realloc(void *mem, size_t new_size)
{
    app_running_thread *thread = appmanager_get_current_thread();
    assert(thread && "invalid thread");

    qrealloc(thread->arena, mem, new_size);
}

uint32_t app_heap_bytes_free(void)
{
    app_running_thread *thread = appmanager_get_current_thread();
    assert(thread && "invalid thread");

    uint32_t free_bytes = qfreebytes(thread->arena);

    return free_bytes;
}

uint32_t app_heap_bytes_used(void)
{
    app_running_thread *thread = appmanager_get_current_thread();
    assert(thread && "invalid thread");

    return qusedbytes(thread->arena);
}
