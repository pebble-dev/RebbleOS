#pragma once
/* snowy_display.h
 * 
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "platform.h"
#include "driver.h"
#define MAX_FRAMEBUFFER_SIZE DISPLAY_ROWS * DISPLAY_COLS

// display command types
#define DISPLAY_CTYPE_NULL        0x00
#define DISPLAY_CTYPE_PARAM       0x01
#define DISPLAY_CTYPE_DISPLAY_OFF 0x02
#define DISPLAY_CTYPE_DISPLAY_ON  0x03
#define DISPLAY_CTYPE_SCENE       0x04
// in full fat mode
#define DISPLAY_CTYPE_FRAME       0x05


typedef struct {
//    SPI *spi;  // SPI6
    GPIO_TypeDef *port_display;
    uint32_t clock_display;
    uint16_t pin_reset;
    uint16_t pin_cs;   
    uint16_t pin_miso;
    uint16_t pin_mosi;
    uint16_t pin_sck;
    
    // inputs
    uint16_t pin_reset_done;
    uint16_t pin_intn;
    
    //state
    uint8_t power_on;   
    uint8_t frame_buffer[DISPLAY_ROWS * DISPLAY_COLS];
} display_t;


void hw_display_init(void);
void hw_display_deinit(void);
int hw_display_test(void);
void hw_display_reset(void);
void hw_display_start(void);
void hw_backlight_init(void);
void hw_backlight_set(uint16_t val);
uint8_t hw_display_is_ready();
uint8_t *hw_display_get_buffer(void);

void hw_display_on();
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset);

// TODO: move to scanline
void scanline_convert_column(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t column_index);
// void scanline_rgb888pixel_to_frambuffer(UG_S16 x, UG_S16 y, UG_COLOR c);

void delay_us(uint16_t us);
void delay_ms(uint16_t ms);


void *hw_display_module_init(hw_driver_handler_t *handler);
