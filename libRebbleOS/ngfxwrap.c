/* ngfxwrap.c
 * Shim between RebbleOS and Neographics
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "context.h"
#include "display.h"

static n_GContext *nGContext;

void rwatch_neographics_init(void)
{
    nGContext = n_graphics_context_from_buffer(display_get_buffer());
}

n_GContext *rwatch_neographics_get_global_context(void)
{
    return nGContext;
}
