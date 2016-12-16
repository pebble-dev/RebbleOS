#include "stm32f4xx.h"
#include "FreeRTOS.h"

#define DISPLAY_RESET     x;
#define DISPLAY SCLK      x;
#define DISPLAY_BL_EN     1; // backlight enable
#define DISPLAY_BL_LVL    2; // backlight level
#define DISPLAY_VIBE_CTL  x;
#define DISPLAY_POWER_CTL x;

typedef struct {
//    SPI *spi;
    
    uint32_t num_rows;
    uint32_t num_cols;
    int32_t num_border_rows;
    int32_t num_border_cols;
    uint8_t row_major;
    uint8_t row_inverted;
    uint8_t col_inverted;
    
    uint8_t          backlight_enabled;
    float         brightness;
    uint8_t power_on;
} display;
