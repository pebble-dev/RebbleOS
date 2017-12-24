#pragma once
/* snowy_ambient,h
 * driver for the Ambient sensor
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "platform.h"

void hw_ambient_init(void);
uint16_t hw_ambient_get(void);
