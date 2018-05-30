#pragma once
/* stm32_cc256x.h
 * Hardware driver for the cc256x chipset
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

const typedef struct {
    stm32_usart_t *usart;
    uint32_t shutdown_power;
    uint32_t shutdown_power_port;
    uint32_t shutdown_pin_num;
    GPIO_TypeDef *shutdown_port;
} stm32_bluetooth_config_t;

static stm32_bluetooth_config_t *_cc256x;

#define STM32_CC256X_MK_IRQ_HANDLERS(usart, dma, txstr, rxstr) \
    STM32_USART_MK_TX_IRQ_HANDLER(usart, dma, txstr, bt_stack_tx_done) \
    STM32_USART_MK_RX_IRQ_HANDLER(usart, dma, rxstr, bt_stack_rx_done)

    
uint8_t stm32_cc256x_init(const stm32_bluetooth_config_t *cc256x);
void stm32_cc256x_clock_on(void);
uint8_t stm32_cc256x_power_cycle(void);
void stm32_cc256x_enable_cts_irq();
void stm32_cc256x_disable_cts_irq(void);
