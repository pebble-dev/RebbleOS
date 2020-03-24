/* snowy_bluetooth.c
 * Hardware driver for the cc256x chipset
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "log.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>
#include "stm32_power.h"
#include "btstack_rebble.h"
#include "snowy_bluetooth.h"
#include "stm32_usart.h"
#include "stm32_cc256x.h"

static const stm32_usart_config_t _usart1_config = {
    .usart                = USART1,
    .flow_control_enabled = FLOW_CONTROL_ENABLED,
    .usart_periph_bus     = STM32_POWER_APB2,
    .gpio_pin_tx_num      = 9,
    .gpio_pin_rx_num      = 10,
    .gpio_pin_rts_num     = 12,
    .gpio_pin_cts_num     = 11,
    .gpio_ptr             = GPIOA,
    .gpio_clock           = RCC_AHB1Periph_GPIOA,
    .usart_clock          = RCC_APB2Periph_USART1,
    .af                   = GPIO_AF_USART1,
    .is_binary            = 1,
};

/* dma tx: dma stream 7 chan 4, rx: stream 2 chan 4. 
 * irq tx: 6, rx: 8 */
static const stm32_dma_t _usart1_dma = STM32_DMA_MK_INIT(RCC_AHB1Periph_DMA2, 2, 7, 2, 4, 4, 6, 8); 

static stm32_usart_t _usart1 = {
    &_usart1_config,
    &_usart1_dma, /* dma */
    115200 /* initial slow handshake */
};


static const stm32_bluetooth_config_t stm32_cc256x_config = {
    .usart                = &_usart1,
    .shutdown_power       = STM32_POWER_AHB1,
    .shutdown_power_port  = RCC_AHB1Periph_GPIOB,
    .shutdown_pin_num     = 12,
    .shutdown_port        = GPIOB,
};    


/* IRQ handlers
 * TX: DMA 2 stream 7.
 * RX: DMA 2 stream 2. */
STM32_CC256X_MK_IRQ_HANDLERS(&_usart1, 2, 7, 2)

uint8_t hw_bluetooth_init(void)
{
    stm32_cc256x_init(&stm32_cc256x_config);
    
    return 0;
}

/* Enable the external 32.768khz clock for the bluetooth.
 * The clock is connected to the Low speed External LSE input
 * It's used to drive RTC and also the bluetooth module.
 * We are going to pass the clock out through MCO1 pin
 */
inline void hw_bluetooth_clock_on(void)
{
    stm32_cc256x_clock_on();
}

/*
 * Request a reset of the bluetooth module
 * Reset the chip by pulling nSHUTD high, sleep then low
 * Then we turn on the ext slow clock 32.768khz clock (MCO1)
 * then issue an HCI reset command.
 * The BT module should give a return of 7 bytes to ACK
 * RTS (Our CTS) will also get pulled LOW to ack reset
 */
inline uint8_t hw_bluetooth_power_cycle(void)
{
    return stm32_cc256x_power_cycle();
}



/*
 * Configure and enable the IRQ for the Clear To Send interrupt
 * The stack will deep sleep bluetooth when idle, and any incoming packets will
 * call this interrupt so we can wake up and get ready
 */
void hw_bluetooth_enable_cts_irq()
{
    stm32_cc256x_enable_cts_irq();
}

/*
 * When we dont need the deep sleep mode, we can ignore the CTS
 */
void hw_bluetooth_disable_cts_irq(void)
{
    stm32_cc256x_disable_cts_irq();
}

stm32_usart_t *hw_bluetooth_get_usart(void)
{
    return &_usart1;
}

