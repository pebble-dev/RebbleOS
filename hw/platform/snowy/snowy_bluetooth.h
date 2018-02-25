#pragma once
#include "btstack_rebble.h"

uint8_t hw_bluetooth_init(void);
void hw_bluetooth_clock_on(void);
uint8_t hw_bluetooth_power_cycle(void);
void hw_bluetooth_set_baud(uint32_t baud);
void hw_bluetooth_set_baud(uint32_t baud);
void hw_bluetooth_disable_cts_irq(void);
void hw_bluetooth_enable_cts_irq(void);
void hw_bluetooth_recv_dma(uint32_t *data, size_t len);
void hw_bluetooth_send_dma(uint32_t *data, uint32_t len);
