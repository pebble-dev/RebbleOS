#include "stm32f4xx.h"
#include "FreeRTOS.h"

#define BUTTON_GPIO GPIOG

typedef struct {
    uint16_t Back;
    uint16_t Up;
    uint16_t Select;
    uint16_t Down;
} buttons_t;

buttons_t buttons = {
    .Back   = GPIO_Pin_4,
    .Up     = GPIO_Pin_3,
    .Select = GPIO_Pin_1,
    .Down   = GPIO_Pin_2,
};
