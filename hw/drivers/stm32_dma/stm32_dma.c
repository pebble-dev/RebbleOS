/* stm_dma.c
 * Implementation of a modular DMA driver
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
#include "stm32_dma.h"
#include "rebble_util.h"

void hw_dma_init(void)
{
    
}

static void _init_dma(stm32_dma_t *dma);

/*
 * initialise the DMA channels for transferring data
 */
void stm32_dma_init_device(stm32_dma_t *dma)
{
    NVIC_InitTypeDef nvic_init_struct;
       
    /* Enable the interrupt for stream copy completion */
    nvic_init_struct.NVIC_IRQChannel = dma->dma_irq_rx_channel;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = dma->dma_irq_rx_pri;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
    
    nvic_init_struct.NVIC_IRQChannel = dma->dma_irq_tx_channel;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = dma->dma_irq_tx_pri;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
}

/*
 * The TX and RX process is broken into 3 parts
 * reset, init and begin.
 * The specific hardware driver that implements this will need
 * to inject their own peripheral dma commands between these stages.
 * The driver can then wrap a full TX or RX request 
 * (see stm32_usart for an example)
 * 
 * NOTE: It is imperitive the driver using DMA turns on the clock
 */
/*
 * Reset the DMA channel and wait for completion
 */
void stm32_dma_tx_disable(stm32_dma_t *dma)
{
    stm32_power_request(STM32_POWER_AHB1, dma->dma_clock);

    DMA_ClearFlag(dma->dma_tx_stream, dma->dma_tx_channel_flags);
    DMA_Cmd(dma->dma_tx_stream, DISABLE);
    while (dma->dma_tx_stream->CR & DMA_SxCR_EN);
    
    stm32_power_release(STM32_POWER_AHB1, dma->dma_clock);
}

/*
 * Initialise the DMA channel, and set the data pointers
 * NOTE: This will not send data yet
 */
void stm32_dma_tx_init(stm32_dma_t *dma, void *periph_address, uint8_t *data, size_t len, uint8_t single_byte)
{
    stm32_power_request(STM32_POWER_AHB1, dma->dma_clock);

    DMA_InitTypeDef dma_init_struct;

    /* Configure DMA controller to manage TX DMA requests */
    DMA_DeInit(dma->dma_tx_stream);
    DMA_ClearFlag(dma->dma_tx_stream, dma->dma_tx_channel_flags);

    DMA_StructInit(&dma_init_struct);
    dma_init_struct.DMA_Channel = dma->dma_tx_channel;
    dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t)(periph_address);
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)data;
    dma_init_struct.DMA_BufferSize = len;
    dma_init_struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init_struct.DMA_MemoryInc =  single_byte == 1 ? DMA_MemoryInc_Disable : DMA_MemoryInc_Enable;
    dma_init_struct.DMA_Mode = DMA_Mode_Normal;
    dma_init_struct.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma_init_struct.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    DMA_Init(dma->dma_tx_stream, &dma_init_struct);

    /* Enable the stream IRQ, periph, DMA and then DMA interrupts in that order */
    NVIC_EnableIRQ(dma->dma_irq_tx_channel);
    DMA_ITConfig(dma->dma_tx_stream, DMA_IT_TC, ENABLE);
}

/*
 * Begin the TX over DMA
 */
void stm32_dma_tx_begin(stm32_dma_t *dma)
{
    DMA_Cmd(dma->dma_tx_stream, ENABLE);
}

/*
 * Reset the RX DMA channel and wait for completion
 */
void stm32_dma_rx_disable(stm32_dma_t *dma)
{
    stm32_power_request(STM32_POWER_AHB1, dma->dma_clock);

    DMA_ClearFlag(dma->dma_rx_stream, dma->dma_rx_channel_flags);
    DMA_Cmd(dma->dma_rx_stream, DISABLE);
    while (dma->dma_rx_stream->CR & DMA_SxCR_EN);

    stm32_power_release(STM32_POWER_AHB1, dma->dma_clock);
}

/*
 * Initialise the DMA channel for RX, and set the data pointers
 * NOTE: This will not receive data yet
 */
void stm32_dma_rx_init(stm32_dma_t *dma, void *periph_addr, uint8_t *data, size_t len)
{
    stm32_power_request(STM32_POWER_AHB1, dma->dma_clock);

    DMA_InitTypeDef dma_init_struct;
    
    /* Configure DMA controller to manage RX DMA requests */
    DMA_DeInit(dma->dma_rx_stream);   
    DMA_ClearFlag(dma->dma_rx_stream, dma->dma_rx_channel_flags);
    
    DMA_StructInit(&dma_init_struct);
    dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t)periph_addr;
    dma_init_struct.DMA_Channel = dma->dma_rx_channel;
    dma_init_struct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)data;
    dma_init_struct.DMA_BufferSize = len;
    dma_init_struct.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_Mode = DMA_Mode_Normal;
    dma_init_struct.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma_init_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(dma->dma_rx_stream, &dma_init_struct);

    NVIC_EnableIRQ(dma->dma_irq_rx_channel);
    DMA_ITConfig(dma->dma_rx_stream, DMA_IT_TC, ENABLE);
}

/*
 * Begin the RX DMA transfer
 */
void stm32_dma_rx_begin(stm32_dma_t *dma)
{
    /* XXX released in IRQ */
    DMA_Cmd(dma->dma_rx_stream, ENABLE);
}

/*
 * IRQ Handler for RX of data complete.
 * The actual IRQ handler is constructed by the implementation 
 * using a MK_XXX_ wrapper.
 */
uint8_t stm32_dma_rx_isr(stm32_dma_t *dma)
{
    if (DMA_GetITStatus(dma->dma_rx_stream, dma->dma_rx_irq_flag) != RESET)
    {
        DMA_ClearITPendingBit(dma->dma_rx_stream, dma->dma_rx_irq_flag);
        return 1;
    }
    return 0;
}

/*
 * IRQ Handler for TX of data complete
 */
uint8_t stm32_dma_tx_isr(stm32_dma_t *dma)
{
    if (DMA_GetITStatus(dma->dma_tx_stream, dma->dma_tx_irq_flag) != RESET)
    {
        DMA_ClearITPendingBit(dma->dma_tx_stream, dma->dma_tx_irq_flag);
        return 1;
        /* Trigger the stack's interrupt handler */
    }
    return 0;
}
