/* stm32_rtc.h
 * STM32F4xx RTC implementation ported to STM32F4xx
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 * Refactored by Thomas England (crc-32) <thomas.c.england@gmail.com>
 */

// make sure we use the external osc
#ifndef __STM32_RTC_H
#define __STM32_RTC_H

#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include <time.h>
#include "rebble_time.h"

void rtc_init(void);
void rtc_config(void);
struct tm *hw_get_time(void);

#endif

#define RTC_CLOCK_SOURCE_LSE
