/* librebble.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include <stdbool.h>
#include "librebble.h"
#include "platform.h"

struct tm *rbl_get_tm(void)
{
    return rebble_time_get_tm();
}

void rbl_draw(void)
{
    display_draw();
}
