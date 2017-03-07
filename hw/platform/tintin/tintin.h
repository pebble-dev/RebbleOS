#ifndef _TINTIN_H
#define _TINTIN_H

#include <sys/types.h>
#include <stdint.h>

#include "stm32f2xx.h"

void debug_init();
void debug_write(const unsigned char *p, size_t len);
void platform_init();
void platform_init_late();

typedef struct {
    uint16_t Pin;
    GPIO_TypeDef *Port;    
} button_t;

/* XXX: probably want to abstract this out, some */
typedef struct {
    button_t Back;
    button_t Up;
    button_t Select;
    button_t Down;
} buttons_t;

void hw_ambient_init();
uint16_t hw_ambient_get();

void hw_backlight_init();
void hw_backlight_set(uint16_t pwm);

void delay_us(uint32_t us);

void hw_buttons_init();
uint8_t hw_button_pressed(uint8_t button_id);

void hw_display_init();
void hw_display_reset();
void hw_display_start();
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset);

#define WATCHDOG_RESET_MS 500
void hw_watchdog_init();
void hw_watchdog_reset();

void rtc_init();
void rtc_config();
void hw_get_time_str(char *buf);

void hw_vibrate_init();
void hw_vibrate_enable(uint8_t enabled);

#endif
