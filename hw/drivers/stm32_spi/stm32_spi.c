/* stm_spi.c
 * Implementation of a modular SPI drivers
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
#    include "stm32f2xx_spi.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include "stdio.h"
#include "stm32_power.h"
#include "debug.h"
#include "log.h"
#include "stm32_spi.h"

static void _spi_init(stm32_spi_config_t *spi);

void stm32_spi_init_device(stm32_spi_t *spi)
{
    _spi_init(spi->config);
    
    if (spi->dma)
    {
        stm32_dma_init_device(spi->dma);
    }
}


/*
 * Intialise the SPI Peripheral
 */
static void _spi_init(stm32_spi_config_t *spi)
{
    GPIO_InitTypeDef gpio_init_struct;
    SPI_InitTypeDef spi_init_struct;

    /* enable clock for used IO pins */
    stm32_power_request(STM32_POWER_AHB1, spi->gpio_clock);

    gpio_init_struct.GPIO_Pin = (1 << spi->gpio_pin_sck_num);
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    

    /* enable peripheral clock */
    stm32_power_request(spi->spi_periph_bus, spi->spi_clock);

    switch(spi->txrx_dir) {
        case STM32_SPI_DIR_RX:
            spi_init_struct.SPI_Direction = SPI_Direction_1Line_Rx; // RX Only. One line

            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_miso_num, spi->af);
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_sck_num, spi->af);
            gpio_init_struct.GPIO_Pin |= (1 << spi->gpio_pin_miso_num);
            break;
        case STM32_SPI_DIR_TX:
            spi_init_struct.SPI_Direction = SPI_Direction_1Line_Tx; // TX Only. One line
            
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_mosi_num, spi->af);
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_sck_num, spi->af);
            gpio_init_struct.GPIO_Pin |= (1 << spi->gpio_pin_mosi_num);
            break;
        case STM32_SPI_DIR_RXTX:
            spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
            
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_miso_num, spi->af);
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_mosi_num, spi->af);
            GPIO_PinAFConfig(spi->gpio_ptr, spi->gpio_pin_sck_num, spi->af);
            gpio_init_struct.GPIO_Pin |= (1 << spi->gpio_pin_miso_num);
            gpio_init_struct.GPIO_Pin |= (1 << spi->gpio_pin_mosi_num);
            break;
        default:
            spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
    }
    
    GPIO_Init(spi->gpio_ptr, &gpio_init_struct);

    stm32_power_release(STM32_POWER_AHB1, spi->gpio_clock);
    
    spi_init_struct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    spi_init_struct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    spi_init_struct.SPI_CPOL = spi->line_polarity;        // clock is low when idle
    spi_init_struct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    spi_init_struct.SPI_NSS = SPI_NSS_Soft;// | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    spi_init_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // SPI frequency is APB2 frequency / 8
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted LSB first
    
    /* any CRC poly? */
    spi_init_struct.SPI_CRCPolynomial = spi->crc_poly;
    SPI_Init(spi->spi, &spi_init_struct); 

    SPI_Cmd(spi->spi, ENABLE);

    stm32_power_release(spi->spi_periph_bus, spi->spi_clock);    
}


/*
 * Request transmission of the buffer provider
 */
void stm32_spi_send_dma(stm32_spi_t *spi, uint32_t *data, size_t len)
{
    /* XXX released in IRQ */
    stm32_power_request(spi->config->spi_periph_bus, spi->config->spi_clock);
    stm32_power_request(STM32_POWER_AHB1, spi->config->gpio_clock);
    stm32_power_request(STM32_POWER_AHB1, spi->dma->dma_clock);
    /* reset the DMA controller ready for tx */
    stm32_dma_tx_reset(spi->dma);
    /* Turn off the SPI DMA for initialisation */
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx, DISABLE);
    /* ready for DMA */
    SPI_Cmd(spi->config->spi, ENABLE);
    stm32_dma_tx_init(spi->dma, (void *)&spi->config->spi->DR, data, len);
    
    /* Turn on our SPI and then the SPI DMA */
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx, ENABLE);

    /* Lets go! */
    stm32_dma_tx_begin(spi->dma);
}


/*
 * Some data arrived from the spi
 */
void stm32_spi_recv_dma(stm32_spi_t *spi, uint32_t *data, size_t len)
{
    DMA_InitTypeDef dma_init_struct;

    stm32_power_request(spi->config->spi_periph_bus, spi->config->spi_clock);
    stm32_power_request(STM32_POWER_AHB1, spi->config->gpio_clock);

    /* reset the DMA controller ready for rx */
    stm32_dma_rx_reset(spi->dma);

    /* init the DMA RX mode */
    SPI_Cmd(spi->config->spi, ENABLE);
    stm32_dma_rx_init(spi->dma, (void *)&spi->config->spi->DR, data, len);

    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Rx, ENABLE);
    stm32_dma_rx_begin(spi->dma);
}

/*
 * IRQ Handler for RX of data complete
 */
void stm32_spi_rx_isr(stm32_spi_t *spi, dma_callback callback)
{
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Rx, DISABLE);
    
    /* release the clocks we are no longer requiring */
    stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
    stm32_power_release(STM32_POWER_AHB1, spi->config->gpio_clock);
    
    /* Trigger the recipient interrupt handler */
    callback();
    
    stm32_power_release(STM32_POWER_AHB1, spi->dma->dma_clock);
}

/*
 * IRQ Handler for TX of data complete
 */
void stm32_spi_tx_isr(stm32_spi_t *spi, dma_callback callback)
{
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx, DISABLE);

    stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
    stm32_power_release(STM32_POWER_AHB1, spi->config->gpio_clock);

    /* Trigger the stack's interrupt handler */
    callback();
    
    stm32_power_release(STM32_POWER_AHB1, spi->dma->dma_clock);
}


void stm32_spi_write(stm32_spi_t *spi, unsigned char c)
{
//     spi->config->spi->DR = c;
//     while (!(spi->config->spi->SR & SPI_SR_TXE))
//         ;
    spi->config->spi->DR = c; // write data to be transmitted to the SPI data register
    while( !(spi->config->spi->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
//     while( !(spi->config->spi->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
//     while( spi->config->spi->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
}
