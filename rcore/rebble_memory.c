/* rebble_memory.c
 * routines for Allocating memory for system and apps. 
 * App memory gets allocated in it's relevant arena heap. System has the rest
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

void rblos_memory_init(void)
{
}

void *system_calloc(size_t count, size_t size)
{
    if (!appmanager_is_thread_system())
        KERN_LOG("main", APP_LOG_LEVEL_DEBUG, "XXX System Calloc. Check who did this");
    
    void *x = pvPortMalloc(count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

void *system_malloc(size_t size)
{
    if (appmanager_is_thread_system())
        KERN_LOG("main", APP_LOG_LEVEL_DEBUG, "XXX System Malloc. Check who did this");
    return pvPortMalloc(size);
}

void *app_malloc(size_t size)
{
    return app_calloc(1, size);    
}

void *app_calloc(size_t count, size_t size)
{
    app_running_thread *thread = appmanager_get_current_thread();

    void *x = qalloc(thread->arena, count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

void app_free(void *mem)
{
    app_running_thread *thread = appmanager_get_current_thread();
    qfree(thread->arena, mem);
}
