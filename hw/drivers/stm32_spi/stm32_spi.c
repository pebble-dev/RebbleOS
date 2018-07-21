/* stm_spi.c
 * Implementation of a modular SPI driver. Not a good one, or a 
 * particularly re-usable one. But it covers off Pebble and STM32
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
#include "platform.h"
static void _spi_init(stm32_spi_config_t *spi);
static uint8_t _stm32_spi_send_recv_dma(stm32_spi_t *spi, uint8_t *outdata, size_t outlen, uint8_t *indata, size_t inlen, uint8_t async, uint8_t single_byte);
uint8_t stm32_spi_poll_wait(stm32_spi_t *spi, uint16_t timeout);

/* comment out to remove *ALL* log output */
// #define SPI_LOG_VERBOSE

#ifdef SPI_LOG_VERBOSE
    #define SPI_LOG DRV_LOG
#else
    #define SPI_LOG NULL_LOG
#endif

#define SPI_TIMEOUT 10000

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
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_DOWN;

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
    
    SPI_Cmd(spi->spi, DISABLE);
    while( spi->spi->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    SPI_DeInit(spi->spi);

    spi_init_struct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    spi_init_struct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    spi_init_struct.SPI_CPOL = spi->line_polarity;        // clock is low when idle
    spi_init_struct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    spi_init_struct.SPI_NSS = SPI_NSS_Soft;// | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    spi_init_struct.SPI_BaudRatePrescaler = spi->spi_prescaler; // SPI frequency is APB2 frequency / 8
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
void stm32_spi_send_dma(stm32_spi_t *spi, uint8_t *data, size_t len)
{
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI Tx %d", len);
    /* XXX released in IRQ */
    stm32_power_request(spi->config->spi_periph_bus, spi->config->spi_clock);

    /* reset the DMA controller ready for tx */
    stm32_dma_tx_disable(spi->dma);
    /* Turn off the SPI DMA for initialisation */
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx, DISABLE);
    stm32_dma_tx_init(spi->dma, (void *)&spi->config->spi->DR, data, len, 0);
    /* Turn on our SPI and then the SPI DMA */
    SPI_Cmd(spi->config->spi, ENABLE);
    /* ready for DMA */
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx, ENABLE);

    /* Lets go! */
    stm32_dma_tx_begin(spi->dma);    
}

/*
 * Read some data from the SPI. Clocks out TX with the dummy char
 * Poll waits sync
 */
uint8_t stm32_spi_recv_dma(stm32_spi_t *spi, uint8_t *data, uint8_t dummy_char, size_t len)
{   
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI TxRx %c %d", dummy_char, len);
    return _stm32_spi_send_recv_dma(spi, &dummy_char, 1, data, len, 0, 1);
}

/*
 * Read some data from the SPI. Clocks out TX with the dummy char
 * Returns immedaitely and calls back via the ISR
 */
uint8_t stm32_spi_recv_dma_async(stm32_spi_t *spi, uint8_t *data, uint8_t dummy_char, size_t len)
{
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI TxRx Async %d %d", dummy_char, len);
    static uint8_t m[1];
    m[0] = dummy_char;
    
    return _stm32_spi_send_recv_dma(spi, m, len, data, len, 1, 1);
}

/*
 * Send and receive data using two buffers, one for each direction
 *   NOTE: unless you know what you are doing, the tx and rx length MUST
 *         be the same length
 * Poll waits sync
 */
uint8_t stm32_spi_send_recv_dma(stm32_spi_t *spi, uint8_t *outdata, size_t outlen, uint8_t *indata, size_t inlen)
{
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI TxRx Sym %d %d", outlen, inlen);
    return _stm32_spi_send_recv_dma(spi, outdata, outlen, indata, inlen, 0, 0);
}

/*
 * Send and receive data using two buffers, one for each direction
 *   NOTE: unless you know what you are doing, the tx and rx length MUST
 *         be the same length
 * Returns immedaitely and calls back via the ISR
 */
uint8_t stm32_spi_send_recv_dma_async(stm32_spi_t *spi, uint8_t *outdata, size_t outlen, uint8_t *indata, size_t inlen)
{
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI TxRx Sym Async %d %d", outlen, inlen);
    return _stm32_spi_send_recv_dma(spi, outdata, outlen, indata, inlen, 1, 0);
}

/* Do the grunt work of an SPI tx/rx transaction */
static uint8_t _stm32_spi_send_recv_dma(stm32_spi_t *spi, uint8_t *outdata, size_t outlen, 
                                        uint8_t *indata, size_t inlen, uint8_t async, uint8_t single_byte)
{
    /* This is unfortunate that we are reserving two clocks here. 
     * One for RX ISR to turn off, one for TX */
    stm32_power_request(spi->config->spi_periph_bus, spi->config->spi_clock);
    stm32_power_request(spi->config->spi_periph_bus, spi->config->spi_clock);
    
    /* reset the DMA controller ready for rx */
    stm32_dma_rx_disable(spi->dma);
    stm32_dma_tx_disable(spi->dma);
    
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);
    
    /* init the DMA RX mode */
    stm32_dma_tx_init(spi->dma, (void *)&spi->config->spi->DR, outdata, outlen, single_byte);
    stm32_dma_rx_init(spi->dma, (void *)&spi->config->spi->DR, indata, inlen);

    SPI_Cmd(spi->config->spi, ENABLE);
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

    if (!async)
    {
        /* Don't generate interrupts here */
        NVIC_DisableIRQ(spi->dma->dma_irq_rx_channel);
        NVIC_DisableIRQ(spi->dma->dma_irq_tx_channel);
    }
    DMA_ITConfig(spi->dma->dma_tx_stream, DMA_IT_TC, async);
    DMA_ITConfig(spi->dma->dma_rx_stream, DMA_IT_TC, async);

    stm32_dma_rx_begin(spi->dma);
    stm32_dma_tx_begin(spi->dma);
    
    /* We are done */
    if (!async)
    {
        uint8_t pw = stm32_spi_poll_wait(spi, SPI_TIMEOUT);
        stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
        stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
        stm32_power_release(STM32_POWER_AHB1, spi->dma->dma_clock);
        stm32_power_release(STM32_POWER_AHB1, spi->dma->dma_clock);
        return pw;
    }
    return 0;
}

/* Poll for the transfer complete flags, then deinit the device */
uint8_t stm32_spi_poll_wait(stm32_spi_t *spi, uint16_t timeout)
{
    /* Now wait for the completion */
    while ((DMA_GetFlagStatus(spi->dma->dma_tx_stream, spi->dma->dma_tx_irq_flag) == RESET) 
            && timeout < SPI_TIMEOUT)
    {
        delay_us(100);
        timeout++;
    };
    timeout = 0;
    while ((DMA_GetFlagStatus(spi->dma->dma_rx_stream, spi->dma->dma_rx_irq_flag) == RESET) 
            && timeout < SPI_TIMEOUT)
    {
        delay_us(100);
        timeout++;
    };
    
    /* Clear DMA Flags */
    stm32_dma_rx_disable(spi->dma);
    stm32_dma_tx_disable(spi->dma);
    
    /* Disable SPI DMA TX + RX Requests */
    SPI_I2S_DMACmd(spi->config->spi, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);

    /* Disable the SPI peripheral */
    SPI_Cmd(spi->config->spi, DISABLE);
 
    SPI_LOG("SPI", APP_LOG_LEVEL_INFO, "SPI DMA Poll Complete");

    return timeout == 0;
}

/*
 * IRQ Handler for RX of data complete
 */
void stm32_spi_rx_isr(stm32_spi_t *spi, dma_callback callback)
{
    stm32_dma_rx_disable(spi->dma);

    /* Trigger the recipient interrupt handler */
    if (callback)
        callback();
    stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
}

/*
 * IRQ Handler for TX of data complete
 */
void stm32_spi_tx_isr(stm32_spi_t *spi, dma_callback callback)
{
    stm32_dma_tx_disable(spi->dma);

    /* Trigger the stack's interrupt handler */
    if (callback)
        callback();
    
    stm32_power_release(spi->config->spi_periph_bus, spi->config->spi_clock);
}

/* Write a single charaction to the TX line unidirectionally. TX ONLY! */
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

/* Do a single charaction SPI transaction (Write + Read) */
uint8_t stm32_spi_write_read(stm32_spi_t *spi, unsigned char c)
{
    while (!(spi->config->spi->SR & SPI_SR_TXE))
        ;
    SPI1->DR = c;
    while (!(spi->config->spi->SR & SPI_SR_RXNE))
        ;
    return spi->config->spi->DR;
}

