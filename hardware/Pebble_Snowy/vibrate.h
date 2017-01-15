#ifndef __VIBRATE_H
#define __VIBRATE_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"

typedef struct {
    uint16_t Pin;
    GPIO_TypeDef *Port;
} vibrate_t;


void vibrate_init(void);
void vibrate_enable(uint8_t enabled);
void vibrate_pattern(uint8_t *pattern, uint8_t patternLength);

#endif
