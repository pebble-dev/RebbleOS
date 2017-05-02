/* rebble_memory.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stdio.h"
#include "string.h"
#include "rebble_memory.h"

// heap4 doesn't have calloc
void *pvPortCalloc(size_t count, size_t size)
{
    void *x = pvPortMalloc(count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

/*
 * Alloc and set memory. Basic checks are done so we don't 
 * hit an exception or ask for stoopid sizes
 */
void *rbl_calloc(size_t count, size_t size)
{
    if (size > xPortGetFreeHeapSize())
    {
        printf("Malloc fail. Not enough heap for %d\n", size);
        return NULL;
    }
    
    if (size > 100000)
    {
        printf("Malloc will fail. > 100Kb requested\n");
        return NULL;
    }
    
    return pvPortCalloc(1, size);
}
