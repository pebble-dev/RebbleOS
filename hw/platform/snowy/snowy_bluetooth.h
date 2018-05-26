#pragma once
#include "btstack_rebble.h"
#include "stm32_usart.h"

uint8_t hw_bluetooth_init(void);
void hw_bluetooth_clock_on(void);
uint8_t hw_bluetooth_power_cycle(void);
void hw_bluetooth_disable_cts_irq(void);
void hw_bluetooth_enable_cts_irq(void);
stm32_usart_t *hw_bluetooth_get_usart(void);
