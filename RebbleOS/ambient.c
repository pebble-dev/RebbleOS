/* ambient.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

void ambient_init(void)
{
    hw_ambient_init();
}

uint16_t ambient_get(void)
{
    // take multiple samples
    // use a simple moving average filter to get the settle
    
    // disable the backlight biefly to get a good clean samples
    // of the real world
    
    return hw_ambient_get();
}
