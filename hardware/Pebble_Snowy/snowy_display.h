#ifndef __SNOWY_DISPLAY_H
#define __SNOWY_DISPLAY_H

#include "stm32f4xx.h"

void hw_display_init(void);
void hw_display_reset(void);
void hw_display_start(void);
void snowy_display_init_intn(void);
void hw_backlight_init(void);
void hw_backlight_set(uint16_t val);
void snowy_display_init_timer(uint16_t pwmValue);
void snowy_display_init_SPI6(void);
void snowy_display_cs(uint8_t enabled);
uint8_t snowy_display_SPI6_getver(uint8_t data);
uint8_t snowy_display_SPI6_send(uint8_t data);
uint8_t snowy_display_FPGA_reset(uint8_t mode);
void snowy_display_reset(uint8_t enabled);
void snowy_display_SPI_start(void);
void snowy_display_SPI_end(void);
void snowy_display_drawscene(uint8_t scene);
void hw_display_on();

void hw_display_send_frame(void);
void snowy_display_start_frame(void);
void snowy_display_send_frame();
uint8_t snowy_display_wait_FPGA_ready(void);
void snowy_display_splash(uint8_t scene);
void snowy_display_full_init(void);
void snowy_display_program_FPGA(void);

void delay_us(uint16_t us);
void delay_ns(uint16_t ns);
#endif
