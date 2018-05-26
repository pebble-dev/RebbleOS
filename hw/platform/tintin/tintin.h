#ifndef _TINTIN_H
#define _TINTIN_H

#include <sys/types.h>
#include <stdint.h>
#include "rebble_time.h"
#include "stm32f2xx.h"
#include "appmanager.h"
#include "flash.h"

extern int printf ( const char* , ... );

void debug_init();
void debug_write(const unsigned char *p, size_t len);
void platform_init();
void platform_init_late();

void hw_ambient_init();
uint16_t hw_ambient_get();

void delay_us(uint32_t us);

void hw_display_init();
void hw_display_reset();
void hw_display_start();
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset);
uint8_t hw_display_get_state();
uint8_t *hw_display_get_buffer(void);

#define WATCHDOG_RESET_MS 500
void hw_watchdog_init();
void hw_watchdog_reset();

void rtc_init();
void rtc_config();
void hw_get_time_str(char *buf);
struct tm *hw_get_time(void);
void rtc_set_timer_interval(TimeUnits tick_units);
void rtc_disable_timer_interval(void);

void hw_vibrate_init();
void hw_vibrate_enable(uint8_t enabled);

void ss_debug_write(const unsigned char *p, size_t len);

void hw_flash_init(void);
void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len);
#define REGION_FPGA_START       0x0
#define REGION_FPGA_SIZE        0x0
#endif
