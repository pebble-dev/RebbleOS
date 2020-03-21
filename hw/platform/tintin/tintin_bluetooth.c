/* tintin_bluetooth.c
 * Tintin configuration routines for controlling a bluetooth stack
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */


#include <stm32f2xx.h>
#include "tintin.h"
#include <debug.h>

#include <stm32f2xx_usart.h>
#include <stm32f2xx_gpio.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_dma.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>
#include "rebbleos.h"

#include "stm32_dma.h"
#include "stm32_usart.h"
#include "stm32_power.h"
#include "stm32_cc256x.h"
#include "btstack_rebble.h"

/* Bluetooth */
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
    115200 /* initial slow handshake. */
};


static const stm32_bluetooth_config_t stm32_cc256x_config = {
    .usart                = &_usart1,
    .shutdown_power       = STM32_POWER_AHB1,
    .shutdown_power_port  = RCC_AHB1Periph_GPIOA,
    .shutdown_pin_num     = 3,
    .shutdown_port        = GPIOA,
};    


/* IRQ handlers
 * TX: DMA 2 stream 7.
 * RX: DMA 2 stream 2. */
STM32_CC256X_MK_IRQ_HANDLERS(&_usart1, 2, 7, 2)

uint8_t hw_bluetooth_init(void)
{
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_USART1);
    stm32_cc256x_init(&stm32_cc256x_config);
    
    return 0;
}

inline void hw_bluetooth_clock_on(void)
{
    stm32_cc256x_clock_on();
}

inline uint8_t hw_bluetooth_power_cycle(void)
{
    return stm32_cc256x_power_cycle();
}

void hw_bluetooth_enable_cts_irq()
{
    stm32_cc256x_enable_cts_irq();
}

void hw_bluetooth_disable_cts_irq(void)
{
    stm32_cc256x_disable_cts_irq();
}

stm32_usart_t *hw_bluetooth_get_usart(void)
{
    return &_usart1;
}
