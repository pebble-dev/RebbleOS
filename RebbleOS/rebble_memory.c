/* rebble_memory.c
 * routines for Allocating memory for system and apps. 
 * App memory gets alocated ont he app's own heap where sys has a global heap
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

// TODO refactor heap allocator and init here
void rblos_memory_init(void)
{
    // init the main heap
    
    // init the app heap
}

// heap4 doesn't have calloc
void *pvPortCalloc(size_t count, size_t size)
{
    KERN_LOG("main", APP_LOG_LEVEL_DEBUG, "System Calloc. Check who did this");
    void *x = pvPortMalloc(count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

// /*
//  * Alloc and set memory. Basic checks are done so we don't 
//  * hit an exception or ask for stoopid sizes
//  */
// void *rbl_calloc(size_t count, size_t size)
// {
//     if (size > xPortGetFreeHeapSize())
//     {
//         printf("Malloc fail. Not enough heap for %d\n", size);
//         while(1);
//         return NULL;
//     }
//     
//     if (size > 100000)
//     {
//         printf("Malloc will fail. > 100Kb requested\n");
//         return NULL;
//     }
//     
//     return pvPortCalloc(1, size);
// }

bool rblos_memory_sanity_check_app(size_t size)
{
    if (size == 0)
    {
        KERN_LOG("main", APP_LOG_LEVEL_ERROR, "Malloc fail. size=0. Huh?");
        return false;
    }
    
    if (size > xPortGetFreeAppHeapSize())
    {
        KERN_LOG("main", APP_LOG_LEVEL_DEBUG, "Malloc fail. Not enough heap for %d", size);
        return false;
        while(1); // TODO for debugging
    }
    
    if (size > 100000)
    {
        KERN_LOG("main", APP_LOG_LEVEL_DEBUG, "Malloc will fail. > 100Kb requested");
        return false;
    }
    
    return true;
}

void *app_malloc(size_t size)
{
    if(!rblos_memory_sanity_check_app(size))
        return NULL;
    
    return pvPortAppMalloc(size);
    
}

void *app_calloc(size_t count, size_t size)
{
    if(!rblos_memory_sanity_check_app(size))
        return NULL;
    
    void *x = pvPortAppMalloc(count * size);
    
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}


void app_free(void *mem)
{
    vPortAppFree(mem);
}
