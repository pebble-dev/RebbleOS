/* librebble.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "librebble.h"
#include "platform.h"
// #include "ugui.h"

struct tm *rbl_get_tm(void)
{
    return rebble_time_get_tm();
}

void rbl_draw(void)
{
    display_draw();
}
