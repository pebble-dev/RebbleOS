/* neographics.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stdio.h"
#include "librebble.h"


n_GContext *nGContext;

void neographics_init(uint8_t *buffer)
{
    
    // Create the global context
    nGContext = n_graphics_context_from_buffer(buffer);
}

n_GContext *neographics_get_global_context(void)
{
    return nGContext;
}
