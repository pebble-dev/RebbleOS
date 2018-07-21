/* tintin_display.c
 * Display driver for Pebble Tintin's SPI display. 
 * {insert hardware here }
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

#include <stm32f2xx.h>
#include "tintin.h"
#include <debug.h>

#include <stm32f2xx_gpio.h>
//#include <stm32f2xx_spi.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>
#include "rebbleos.h"
#include "stm32_power.h"
#include "stm32_spi.h"

/* comment out of you don't want DMA */
#define DMA_ENABLED

/* How many rows do we want to send at once 
 * NOTE: This is going to use more buffer ram the biggger you go
 * also make sure its divisible by 168 lest you be doomed to 
 * sending too many bytes and having to fix it.
 */
#define _DMA_ROW_COUNT 8

static const stm32_spi_config_t _spi2_config = {
    .spi                  = SPI2,
    .spi_periph_bus       = STM32_POWER_APB1,
    .gpio_pin_miso_num    = 0,
    .gpio_pin_mosi_num    = 15,
    .gpio_pin_sck_num     = 13,
    .gpio_ptr             = GPIOB,
    .gpio_clock           = RCC_AHB1Periph_GPIOB,
    .spi_clock            = RCC_APB1Periph_SPI2,
    .af                   = GPIO_AF_SPI2,
    .txrx_dir             = STM32_SPI_DIR_TX,
    .spi_prescaler        = SPI_BaudRatePrescaler_8,
    .crc_poly             = 7,
    .line_polarity        = SPI_CPOL_Low
};

static const stm32_dma_t _spi2_dma = {
    .dma_clock            = RCC_AHB1Periph_DMA1,
    .dma_tx_stream        = DMA1_Stream4,
    .dma_rx_stream        = DMA1_Stream3,
    .dma_tx_channel       = DMA_Channel_0,
    .dma_rx_channel       = DMA_Channel_0,
    .dma_irq_tx_pri       = 10,
    .dma_irq_rx_pri       = 9,
    .dma_irq_tx_channel   = DMA1_Stream4_IRQn,
    .dma_irq_rx_channel   = DMA1_Stream3_IRQn,
    .dma_tx_channel_flags = STM32_DMA_MK_FLAGS(4),
    .dma_rx_channel_flags = STM32_DMA_MK_FLAGS(3),
    .dma_tx_irq_flag      = DMA_IT_TCIF4,
    .dma_rx_irq_flag      = DMA_IT_TCIF3
};

static stm32_spi_t _spi2 = {
    &_spi2_config,
#ifdef DMA_ENABLED
    &_spi2_dma, /* dma */
#else
    NULL
#endif
};

static void _send_next(uint32_t row);
static void _spi_tx_done(void);

/* TX ISR for DMA */
STM32_SPI_MK_TX_IRQ_HANDLER(&_spi2, 1, 4, _spi_tx_done)

#define DISPLAY_CLOCK 1
#define DISPLAY_CS    12

#define DISPLAY_FRAME_START 0x80

/* display */

static uint8_t _display_fb[168][20];
void hw_display_start_frame_dma(uint8_t x, uint8_t y);

void hw_display_init() {
    DRV_LOG("Display", APP_LOG_LEVEL_INFO, "tintin: hw_display_init");

    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);

    GPIO_WriteBit(GPIOB, 1 << DISPLAY_CS, 0);
    GPIO_PinAFConfig(GPIOB, 1, GPIO_AF_TIM3);
   
    GPIO_InitTypeDef gpioinit;
    
    gpioinit.GPIO_Pin = (1 << DISPLAY_CS);
    gpioinit.GPIO_Mode = GPIO_Mode_OUT;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpioinit);
    
    gpioinit.GPIO_Pin = (1 << DISPLAY_CLOCK);
    gpioinit.GPIO_Mode = GPIO_Mode_AF;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_OD;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpioinit);

    /* Set up the SPI controller, SPI2. */
    stm32_spi_init_device(&_spi2);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
}

void hw_display_reset() {
    DRV_LOG("Display", APP_LOG_LEVEL_INFO, "tintin: hw_display_reset");
}

void hw_display_start() {
    DRV_LOG("Display", APP_LOG_LEVEL_INFO, "tintin: hw_display_start");
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
#ifdef DMA_ENABLED
    hw_display_start_frame_dma(x, y);
    return;
#else
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_SPI2);

    DRV_LOG("Display", APP_LOG_LEVEL_DEBUG, "tintin: here we go, slowly blitting %d %d", x, y);
    GPIO_WriteBit(GPIOB, 1 << DISPLAY_CS, 1);
    delay_us(7);
    stm32_spi_write(&_spi2, DISPLAY_FRAME_START);
    for (int i = 0; i < 168; i++) {
        stm32_spi_write(&_spi2, __RBIT(__REV(168-i)));
        for (int j = 0; j < 18; j++)
            stm32_spi_write(&_spi2, _display_fb[i][17-j]);
        stm32_spi_write(&_spi2, 0);
    }
    stm32_spi_write(&_spi2, 0);
    delay_us(7);
    GPIO_WriteBit(GPIOB, 1 << DISPLAY_CS, 0);

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_SPI2);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);

    display_done_ISR(0);
#endif
}

void hw_display_start_frame_dma(uint8_t x, uint8_t y) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_SPI2);

//     DRV_LOG("Display", APP_LOG_LEVEL_DEBUG, "tintin: Display Yeeehaw (DMA) %d %d", x, y);
    GPIO_WriteBit(GPIOB, 1 << DISPLAY_CS, 1);
    delay_us(10);
    stm32_spi_write(&_spi2, DISPLAY_FRAME_START);
    
    /* Start the transfer */
    _send_next(0);
}

static void _send_next(uint32_t row)
{
    static uint8_t row_buf[20 * _DMA_ROW_COUNT];
    uint16_t len = 20 * _DMA_ROW_COUNT;
    
    for (int i = 0; i < _DMA_ROW_COUNT; i++)
    {
        uint32_t rp = i * 20;
        row_buf[rp] = __RBIT(__REV(168 - row - i));
        for (int j = 0; j < 18; j++) {
            row_buf[rp + j + 1] = _display_fb[row + i][17-j];
        }
        row_buf[rp + 19] = 0;
    }

    stm32_spi_send_dma(&_spi2, row_buf, len);
}

uint8_t *hw_display_get_buffer(void) {
    return (uint8_t *)_display_fb;
}

uint8_t hw_display_get_state() {
    return 1;
}

static void _spi_tx_done(void)
{
    static uint8_t row_index = 0;
    
    if (row_index < 168 - _DMA_ROW_COUNT)
    {
        row_index += _DMA_ROW_COUNT;
        _send_next(row_index);
        return;
    }
    row_index = 0;

    /* if we are finished sending each column, then reset and stop */
    stm32_spi_write(&_spi2, 0);
    delay_us(7);
    GPIO_WriteBit(GPIOB, 1 << DISPLAY_CS, 0);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_SPI2);
    
    display_done_ISR(0);
}

