#pragma once

#include "btstack_run_loop_freertos.h"
#include "stm32_usart.h"

void port_main(void);
void bt_device_init(void);
void bt_device_request_tx(uint8_t *data, uint16_t len);
void bluetooth_power_cycle(void);
void bt_stack_tx_done();
void bt_stack_rx_done();
void bt_stack_cts_irq();

stm32_usart_t *hw_bluetooth_get_usart(void);

#define BLUETOOTH_MODULE_TYPE_NONE    0
#define BLUETOOTH_MODULE_TYPE_CC2564  1
#define BLUETOOTH_MODULE_TYPE_CC2564B 2
