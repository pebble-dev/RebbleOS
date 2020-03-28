/*
 * stm32_usart.h
 * External-facing API for stm32 USART module
 * This is a simple wrapper around common USART defaults and functions
 * RebbleOS
 *
 * Barry Carter <barry.carter@gmail.com>
 *
 */
#pragma once
#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_gpio.h"
#    include "stm32f2xx_usart.h"
#    include "stm32f2xx_rcc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif

#include <stdint.h>
#include "stm32_dma.h"

#define FLOW_CONTROL_DISABLED 0
#define FLOW_CONTROL_ENABLED  1

const typedef struct {
    USART_TypeDef *usart;
    uint8_t flow_control_enabled;
    uint32_t usart_periph_bus;
    uint32_t gpio_pin_tx_num;
    uint32_t gpio_pin_rx_num;
    uint32_t gpio_pin_rts_num;
    uint32_t gpio_pin_cts_num;
    GPIO_TypeDef *gpio_ptr;
    uint32_t gpio_clock;
    uint32_t usart_clock;
    uint32_t af;
    int is_binary;
} stm32_usart_config_t;

typedef struct {
    const stm32_usart_config_t *config;
    const stm32_dma_t *dma;
    uint32_t baud;
} stm32_usart_t;

void stm32_usart_init_device(stm32_usart_t *usart);
void stm32_usart_set_baud(stm32_usart_t *usart, uint32_t baud);
size_t stm32_usart_write(stm32_usart_t *usart, const uint8_t *buf, size_t len);
size_t stm32_usart_read(stm32_usart_t *usart, uint8_t *buf, size_t len);
void stm32_usart_send_dma(stm32_usart_t *usart, uint32_t *data, size_t len);
void stm32_usart_recv_dma(stm32_usart_t *usart, uint32_t *data, size_t len);

void stm32_usart_tx_isr(stm32_usart_t *usart, dma_callback callback);
void stm32_usart_rx_isr(stm32_usart_t *usart, dma_callback callback);

static inline void _stm32_usart_tx_isr(void);
static inline void _stm32_usart_rx_isr(void);
void stm32_usart_rx_start_init_isr(void);
void stm32_usart_rx_irq_enable(stm32_usart_t *usart, uint8_t enabled);
void stm32_usart_rx_start_isr(stm32_usart_t *usart, dma_callback callback);
void _stm32_usart_rx_start_init_isr(stm32_usart_t *usart, uint16_t irqn, uint16_t rx_pri);

#define STM32_USART_MK_TX_IRQ_HANDLER(usart, dma_channel, stream, callback) \
    STM32_DMA_MK_TX_IRQ_HANDLER( (usart) ->dma, dma_channel, stream, _stm32_usart_tx_isr ) \
    \
    static inline void _stm32_usart_tx_isr(void) { \
        stm32_usart_tx_isr(usart, callback); \
    }

#define STM32_USART_MK_RX_IRQ_HANDLER(usart, dma_channel, stream, callback) \
    STM32_DMA_MK_RX_IRQ_HANDLER( (usart) ->dma, dma_channel, stream, _stm32_usart_rx_isr ) \
    \
    static inline void _stm32_usart_rx_isr(void) { \
        stm32_usart_rx_isr(usart, callback); \
    }

#define STM32_USART_MK_RX_START_IRQ_HANDLER(usart, usart_num, rx_pri, callback) \
    void stm32_usart_rx_start_init_isr(void) { \
        _stm32_usart_rx_start_init_isr(usart, USART ## usart_num ## _IRQn, rx_pri);\
    } \
    void USART ## usart_num ## _IRQHandler(void) { \
        stm32_usart_rx_start_isr(usart, callback); \
    }
