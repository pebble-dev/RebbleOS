/* 
 * stm32_spi.h
 * External-facing API for stm32 SPI module
 * This is a simple wrapper around common SPI defaults and functions
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
#    include "stm32f2xx_dma.h"
#    include "stm32f2xx_spi.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif

#include <stdint.h>
#include "stm32_dma.h"

#define STM32_SPI_DIR_RXTX 0
#define STM32_SPI_DIR_RX   1
#define STM32_SPI_DIR_TX   2

const typedef struct {
    SPI_TypeDef *spi;
    uint32_t spi_periph_bus;
    uint32_t gpio_pin_miso_num;
    uint32_t gpio_pin_mosi_num;
    uint32_t gpio_pin_sck_num;
    GPIO_TypeDef *gpio_ptr;
    uint32_t gpio_clock;
    uint32_t spi_clock;
    uint32_t af;
    uint8_t txrx_dir;
    uint16_t crc_poly;
    uint8_t line_polarity;
} stm32_spi_config_t;



typedef struct {
    const stm32_spi_config_t *config;
    const stm32_dma_t *dma;
} stm32_spi_t;


static inline void _stm32_spi_tx_isr(void);
static inline void _stm32_spi_rx_isr(void);
#define STM32_SPI_MK_TX_IRQ_HANDLER(spi, dma_channel, stream, callback) \
    STM32_DMA_MK_TX_IRQ_HANDLER( (spi) ->dma, dma_channel, stream, _stm32_spi_tx_isr ) \
    \
    static inline void _stm32_spi_tx_isr(void) { \
        stm32_spi_tx_isr(spi, callback); \
    }
    
#define STM32_SPI_MK_RX_IRQ_HANDLER(spi, dma_channel, stream, callback) \
    STM32_DMA_MK_RX_IRQ_HANDLER( (spi) ->dma, dma_channel, stream, _stm32_spi_rx_isr ) \
    \
    static inline void _stm32_spi_rx_isr(void) { \
        stm32_spi_rx_isr(spi, callback); \
    }

void stm32_spi_init_device(stm32_spi_t *spi);
void stm32_spi_send_dma(stm32_spi_t *spi, uint32_t *data, size_t len);
void stm32_spi_recv_dma(stm32_spi_t *spi, uint32_t *data, size_t len);
void stm32_spi_write(stm32_spi_t *spi, unsigned char c);

void stm32_spi_rx_isr(stm32_spi_t *spi, dma_callback callback);
void stm32_spi_tx_isr(stm32_spi_t *spi, dma_callback callback);
