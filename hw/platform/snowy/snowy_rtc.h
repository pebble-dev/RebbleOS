/* snowy_rtc.h
 * STM32F4xx RTC implementation
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <time.h>
#include "rebble_time.h"

void rtc_init(void);
void rtc_config(void);
void hw_get_time_str(char *buf);
struct tm *hw_get_time(void);

// make sure we use the external osc
#define RTC_CLOCK_SOURCE_LSE

void rtc_set_timer_interval(TimeUnits tick_units);
void rtc_disable_timer_interval(void);
