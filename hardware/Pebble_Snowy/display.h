#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"

/*display_done: G9 (in)
display_intn: G10 (in)
display_reset: G15 (out)
display_sclk: G13 (out)

*/

typedef struct {
//    SPI *spi;  // SPI6
    GPIO_TypeDef *PortDisplay;
    uint16_t PinReset;
    uint16_t PinPower;
    uint16_t PinCs;
    uint16_t PinBacklight;
    GPIO_TypeDef *PortBacklight;
    uint16_t PinVibrate;
    GPIO_TypeDef *PortVibrate;
    
    uint16_t PinMiso;
    uint16_t PinMosi;
    uint16_t PinSck;
    
    // inputs
    uint16_t PinDone;
    uint16_t PinIntn;
    
    
    // stuff from qemu
    uint32_t num_rows;
    uint32_t num_cols;
    int32_t num_border_rows;
    int32_t num_border_cols;
    uint8_t row_major;
    uint8_t row_inverted;
    uint8_t col_inverted;
    
    
    //state
    uint8_t BacklightEnabled;
    float   Brightness;
    uint8_t PowerOn;
} display_t;



void display_init(void);
void display_backlight(uint8_t enabled);
void display_vibrate(uint8_t enabled);

void display_test(uint8_t scene);

#endif
