#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "display.h"


display_t display = {
    .PinReset = GPIO_Pin_15,
    .PinPower = GPIO_Pin_1,
    .PinBacklight = GPIO_Pin_14,
    .PortBacklight = GPIOB,
    .PinVibrate = GPIO_Pin_4,
    .PortVibrate = GPIOF,
    
    .num_rows = 0,
    .num_cols = 0,
    .num_rows = 0,
    .num_border_rows = 0,
    .num_border_cols = 0,
    .row_major = 0,
    .row_inverted = 0,
    .col_inverted = 0,
    
    .BacklightEnabled = 0,
    .Brightness = 0,
    .PowerOn = 0,
};

void display_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure_Vibr;
    
    // init the backlight
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = display.PinBacklight;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortBacklight, &GPIO_InitStructure);
    
    // Timer 12 is also used to generate the brightness
    
    
    // init the vibrator
    GPIO_InitStructure_Vibr.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Vibr.GPIO_Pin = display.PinVibrate;
    GPIO_InitStructure_Vibr.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Vibr.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Vibr.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortVibrate, &GPIO_InitStructure_Vibr);
}

void display_backlight(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortBacklight, display.PinBacklight);
    else
        GPIO_ResetBits(display.PortBacklight, display.PinBacklight);
}

void display_vibrate(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortVibrate, display.PinVibrate);
    else
        GPIO_ResetBits(display.PortVibrate, display.PinVibrate);
}
