#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "vibrate.h"
#include "task.h"
#include "semphr.h"
#include <stm32f4xx_spi.h>

//void vVibratePatternTask(void *pvParameters);
//static TaskHandle_t xVibratePatternTask;


vibrate_t vibrate = {
    .Pin     = GPIO_Pin_4,
    .Port    = GPIOF,
};

void vibrate_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure_Vibr;
    
    // init the vibrator
    GPIO_InitStructure_Vibr.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Vibr.GPIO_Pin = vibrate.Pin;
    GPIO_InitStructure_Vibr.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Vibr.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Vibr.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(vibrate.Port, &GPIO_InitStructure_Vibr);
}


void vibrate_enable(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(vibrate.Port, vibrate.Pin);
    else
        GPIO_ResetBits(vibrate.Port, vibrate.Pin);
}

void vibrate_pattern(uint8_t *pattern, uint8_t patternLength)
{
    // set the timer 12 CH2 to pwm to vibrate
}
