#pragma once
/* snowy,h
 * Various platform utils
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include <sys/types.h>

// How often are we resetting the watchdog timer (ms)
#define WATCHDOG_RESET_MS 500

// main hooks
void debug_init(void);
void debug_write(const unsigned char *p, size_t len);
void ss_debug_write(const unsigned char *p, size_t len);
void platform_init(void);
void platform_init_late(void);

// internal
void init_USART3(void);
void init_USART8(void);


// implementation
void hw_watchdog_init(void);
void hw_watchdog_reset(void);
