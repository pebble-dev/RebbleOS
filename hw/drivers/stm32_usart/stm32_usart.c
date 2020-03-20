/* stm_usart.c
 * Implementation of a modular usart driver
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_gpio.h"
#    include "stm32f2xx_dma.h"
#    include "stm32f2xx_syscfg.h"
#    include "stm32f2xx_rcc.h"
#    include "stm32f2xx_usart.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include "stdio.h"
#include "stm32_power.h"
#include "debug.h"
#include "log.h"
#include "stm32_usart.h"

#define USART_FLOW_CONTROL_DISABLED 0
#define USART_FLOW_CONTROL_ENABLED  1

static void _usart_init(stm32_usart_t *usart);

void stm32_usart_init_device(stm32_usart_t *usart)
{
    _usart_init(usart);
    
    if (usart->dma)
    {
        stm32_dma_init_device(usart->dma);
    }
}


/*
 * Intialise the USART
 * 
 */
static void _usart_init(stm32_usart_t *usart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    assert(usart && usart->config && "Please configure your usart");
    stm32_usart_config_t *u = usart->config;

    stm32_power_request(u->usart_periph_bus, u->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, u->gpio_clock);
    
    GPIO_InitStruct.GPIO_Pin = (1 << u->gpio_pin_tx_num) | 
                               (1 << u->gpio_pin_rx_num);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(usart->config->gpio_ptr, &GPIO_InitStruct);
    
    if (u->flow_control_enabled)
    {
        /* Enable AF on CTS & RTS */
        GPIO_InitStruct.GPIO_Pin = (1 << u->gpio_pin_cts_num) | 
                                   (1 << u->gpio_pin_rts_num);

        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(usart->config->gpio_ptr, &GPIO_InitStruct);
    }
    
    USART_DeInit(u->usart);
    USART_StructInit(&USART_InitStruct);

    USART_InitStruct.USART_BaudRate = usart->baud;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    GPIO_PinAFConfig(u->gpio_ptr, u->gpio_pin_tx_num, u->af);
    GPIO_PinAFConfig(u->gpio_ptr, u->gpio_pin_rx_num, u->af);

    if (u->flow_control_enabled)
    {
        /* AF for USART with hardware flow control */
        GPIO_PinAFConfig(u->gpio_ptr, u->gpio_pin_cts_num, u->af);
        GPIO_PinAFConfig(u->gpio_ptr, u->gpio_pin_rts_num, u->af);
        
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
    }
    else
    {
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    }

    stm32_usart_rx_start_init_isr();

    USART_Init(u->usart, &USART_InitStruct);
    
    /* USART is ready to go */
    USART_Cmd(u->usart, ENABLE);
    
    stm32_power_release(u->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
}

__attribute__((weak))
void stm32_usart_rx_start_init_isr(void) 
{

}

void _stm32_usart_rx_start_init_isr(stm32_usart_t *usart, uint16_t irqn, uint16_t rx_pri)
{
    NVIC_InitTypeDef nvic_init_struct;

    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);

    /* Enable the USART RX Interrupt */
    USART_ITConfig(usart->config->usart, USART_IT_RXNE, ENABLE);
    nvic_init_struct.NVIC_IRQChannel = irqn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = rx_pri;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);

    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
}


void stm32_usart_rx_irq_enable(stm32_usart_t *usart, uint8_t enabled)
{
    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);

    /* Enable the USART RX Interrupt */
    USART_ITConfig(usart->config->usart, USART_IT_RXNE, enabled);

    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
}

/*
 * IRQ Handler for RX started
 */
void stm32_usart_rx_start_isr(stm32_usart_t *usart, dma_callback callback)
{
    if (USART_GetITStatus(usart->config->usart, USART_IT_RXNE) != RESET)
    {
        /* Trigger the recipient interrupt handler */
        callback();
    }
}


/*
 * Request transmission of the buffer provider
 */
void stm32_usart_send_dma(stm32_usart_t *usart, uint32_t *data, size_t len)
{
    /* XXX released in IRQ */
    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->dma->dma_clock);
    /* reset the DMA controller ready for tx */
    stm32_dma_tx_disable(usart->dma);
    /* Turn off the USART DMA for initialisation */
    USART_DMACmd(usart->config->usart, USART_DMAReq_Tx, DISABLE);
    /* ready for DMA */
    stm32_power_release(STM32_POWER_AHB1, usart->dma->dma_clock);
    stm32_dma_tx_init(usart->dma, (void *)(&usart->config->usart->DR), (uint8_t *)data, len, 0);
    
    /* Turn on our USART and then the USART DMA */
    USART_Cmd(usart->config->usart, ENABLE);
    USART_DMACmd(usart->config->usart, USART_DMAReq_Tx, ENABLE);
    /* Lets go! */
    stm32_dma_tx_begin(usart->dma);
}

/*
 * Ready the USART for DMA RX transfer
 */
void stm32_usart_recv_dma(stm32_usart_t *usart, uint32_t *data, size_t len)
{
    DMA_InitTypeDef dma_init_struct;

    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);
    /* we have to control the DMA clock */
    stm32_power_request(STM32_POWER_AHB1, usart->dma->dma_clock);

    /* reset the DMA controller ready for rx */
    stm32_dma_rx_disable(usart->dma);
    stm32_power_release(STM32_POWER_AHB1, usart->dma->dma_clock);
    USART_DMACmd(usart->config->usart, USART_DMAReq_Rx, DISABLE);
    
    /* init the DMA RX mode */
    stm32_dma_rx_init(usart->dma, (void *)&usart->config->usart->DR, (uint8_t *)data, len);
    USART_Cmd(usart->config->usart, ENABLE);

    USART_DMACmd(usart->config->usart, USART_DMAReq_Rx, ENABLE);
    stm32_dma_rx_begin(usart->dma);
}

/* 
 * Set or change the baud rate of the USART
 * This is safe to be done any time there is no transaction in progress
 */
void stm32_usart_set_baud(stm32_usart_t *usart, uint32_t baud)
{
    DRV_LOG("dma", APP_LOG_LEVEL_DEBUG, "Baud");
    usart->baud = baud;
    _usart_init(usart);
}

/*
 * IRQ Handler for RX of data complete
 */
void stm32_usart_rx_isr(stm32_usart_t *usart, dma_callback callback)
{
    USART_DMACmd(usart->config->usart, USART_DMAReq_Rx, DISABLE);
    
    /* release the clocks we are no longer requiring */
    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
    
    /* Trigger the recipient interrupt handler */
    callback();
}

/*
 * IRQ Handler for TX of data complete
 */
void stm32_usart_tx_isr(stm32_usart_t *usart, dma_callback callback)
{
    USART_DMACmd(usart->config->usart, USART_DMAReq_Tx, DISABLE);

    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);

    /* Trigger the stack's interrupt handler */
    callback();
}

uint8_t _timeout(volatile uint32_t reg, uint16_t timeout)
{
    uint16_t _timeout = 0;
    while (!(reg))
    {
        _timeout++;
        if (_timeout > timeout)
        {
            DRV_LOG("USART", APP_LOG_LEVEL_DEBUG, "Timed Out");
            return 1;
        }
    }
    return 0;
}

/* Util function to directly read and write the USART */
size_t stm32_usart_write(stm32_usart_t *usart, const uint8_t *buf, size_t len)
{
    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);

    int i;
    for (i = 0; i < len; i++)
    {
        //while (!(usart->config->usart->SR & USART_SR_TC));
        if (_timeout(usart->config->usart->SR & USART_SR_TC, 10))
            break;
        USART_SendData(usart->config->usart, ((uint8_t *) buf)[i]);
    }
    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
    
    return i;
}

size_t stm32_usart_read(stm32_usart_t *usart, uint8_t *buf, size_t len)
{
    int i;
    
    stm32_power_request(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_request(STM32_POWER_AHB1, usart->config->gpio_clock);

    for (i = 0; i < len; i++)
    {
//         while (!(usart->config->usart->SR & USART_FLAG_RXNE));
        if (_timeout(usart->config->usart->SR & USART_FLAG_RXNE, 10))
            break;
        ((uint8_t *) buf)[i] = USART_ReceiveData(usart->config->usart);
    }

    stm32_power_release(usart->config->usart_periph_bus, usart->config->usart_clock);
    stm32_power_release(STM32_POWER_AHB1, usart->config->gpio_clock);
    
    return i;
}
