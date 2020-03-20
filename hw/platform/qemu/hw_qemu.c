#if defined(STM32F2XX) || defined(STM32F4XX)

#include "hw_qemu.h"
#include "stm32_usart.h"
#include "stm32_power.h"

void _hw_qemu_rx(void);

static const stm32_usart_config_t _usart2_config = {
    .usart                = USART2,
    .flow_control_enabled = FLOW_CONTROL_DISABLED,
    .usart_periph_bus     = STM32_POWER_APB1,
    .gpio_pin_tx_num      = 2,
    .gpio_pin_rx_num      = 3, // this is even more ignored by QEMU than tx
    .gpio_pin_rts_num     = 0,
    .gpio_pin_cts_num     = 0,
    .gpio_ptr             = GPIOA,
    .gpio_clock           = RCC_AHB1Periph_GPIOA,
    .usart_clock          = RCC_APB1Periph_USART2,
    .af                   = GPIO_AF_USART2,
    .is_binary            = 1,
};

extern void qemu_rx_started_isr(void);

static stm32_usart_t _usart2 = {
    &_usart2_config,
    NULL, /* no dma */
    230400,
};

STM32_USART_MK_RX_START_IRQ_HANDLER(&_usart2, 2, 6, &_hw_qemu_rx);

void hw_qemu_init(void)
{
    stm32_usart_init_device(&_usart2);
}

void _hw_qemu_rx(void)
{
    stm32_usart_rx_irq_enable(&_usart2, DISABLE);
    qemu_rx_started_isr();
}

size_t hw_qemu_read(void *buffer, size_t max_len)
{
    size_t read_bytes = stm32_usart_read(&_usart2, (uint8_t*)buffer, max_len);
    return read_bytes;
}

size_t hw_qemu_write(const void *buffer, size_t len)
{
    return stm32_usart_write(&_usart2, (const uint8_t*)buffer, len);
}

void hw_qemu_irq_enable(void)
{
    stm32_usart_rx_irq_enable(&_usart2, ENABLE);
}

#else

#include <stddef.h>

void _hw_qemu_rx(void);

void hw_qemu_init(void)
{
}

void _hw_qemu_rx(void)
{
}

size_t hw_qemu_read(void *buffer, size_t max_len)
{
    return 0;
}

size_t hw_qemu_write(const void *buffer, size_t len)
{
    return len;
}

void hw_qemu_irq_enable(void)
{
}

#endif
