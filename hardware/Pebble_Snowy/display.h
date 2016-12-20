#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"

/*display_done: G9 (in)
display_intn: G10 (in)
display_reset: G15 (out)
display_sclk: G13 (out)

*/

#define DISPLAY_MODE_BOOTLOADER      0
#define DISPLAY_MODE_FULLFAT         1

// todo TYPEDEF ENUM
#define DISPLAY_CMD_IDLE       0
#define DISPLAY_CMD_INIT       1
#define DISPLAY_CMD_RESET      2
#define DISPLAY_CMD_DISPLAY_ON 3
#define DISPLAY_CMD_BEGIN      4
#define DISPLAY_CMD_DRAW       5
#define DISPLAY_CMD_FLASH      6
#define DISPLAY_CMD_INITF      7

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
    uint32_t NumRows;
    uint32_t NumCols;
    int32_t num_border_rows;
    int32_t num_border_cols;
    uint8_t row_major;
    uint8_t row_inverted;
    uint8_t col_inverted;
    
    
    //state
    uint8_t BacklightEnabled;
    float   Brightness;
    uint8_t PowerOn;
    uint8_t DisplayState; // busy etc
    uint8_t DisplayMode; // bootloader or full
    
    char DisplayBuffer[24192];
} display_t;



void display_init(void);
void display_backlight(uint8_t enabled);
void display_vibrate(uint8_t enabled);

void display_test(uint8_t scene);

void display_cmd(uint8_t cmd, char *data);

#endif
