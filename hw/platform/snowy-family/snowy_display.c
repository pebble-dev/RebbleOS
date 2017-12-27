/* snowy_display.c
 * Color display over FPGA implementation for Pebble Time (snowy)
 * Start frame is called, which processed, and then yields to the master isr
 * 
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "log.h"
#include "vibrate.h"
#include "snowy_display.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>
#include "stm32_power.h"

#define ROW_LENGTH    DISPLAY_COLS
#define COLUMN_LENGTH DISPLAY_ROWS
static uint8_t _column_buffer[COLUMN_LENGTH];
static uint8_t _display_ready;

void _snowy_display_start_frame(uint8_t xoffset, uint8_t yoffset);
uint8_t _snowy_display_wait_FPGA_ready(void);
void _snowy_display_splash(uint8_t scene);
void _snowy_display_full_init(void);
void _snowy_display_program_FPGA(void);
void _snowy_display_send_frame(void);
void _snowy_display_init_SPI6(void);
void _snowy_display_cs(uint8_t enabled);
uint8_t _snowy_display_SPI6_getver(uint8_t data);
uint8_t _snowy_display_SPI6_send(uint8_t data);
uint8_t _snowy_display_FPGA_reset(uint8_t mode);
void _snowy_display_reset(uint8_t enabled);
void _snowy_display_SPI_start(void);
void _snowy_display_SPI_end(void);
void _snowy_display_drawscene(uint8_t scene);
void _snowy_display_init_intn(void);
void _snowy_display_dma_send(uint8_t *data, uint32_t len);
void _snowy_display_next_column(uint8_t col_index);
void _snowy_display_init_dma(void);

// pointer to the place in flash where the FPGA image resides
extern unsigned char _binary_Resources_FPGA_4_3_snowy_dumped_bin_start;
extern unsigned char _binary_Resources_FPGA_4_3_snowy_dumped_bin_size;

/*
 * Generic functional notes 
 * 
 * Bootloader:
 *  The pebble bootloader flash is a basic scene selection arrangement.
 *  (It is not implemented here)
 *  The bootloader on the Pebble device will initialise the FPGA
 *  First it checks to see if there is an updated version of the 
 *  display's FPGA code on the flash at 0xF0004
 *  The first two bytes at 0xF0000 are the FPGA ROM size.
 *  
 *  If this fails, it falls back to using the NVCM (Non-Volatile Configuration Memory)
 *  on the FPGA. This is known to have bugs, hence the on chip bootloader update.
 *
 * Main Image
 *  The Pebble firmware itself contains another FPGA flash that has a raw draw mode
 *  On init, it resets the FPGA into "programming" mode, and uploads the image.
 * 
 *  There is one known command for this ROM, "Draw frame". Which it duly does.
 */


// Display configuration for the Pebble TIME
display_t display = {
    .port_display    = GPIOG,
    .clock_display   = RCC_AHB1Periph_GPIOG,
    .pin_reset       = GPIO_Pin_15,
    .pin_cs          = GPIO_Pin_8, 
    .pin_miso        = GPIO_Pin_12,
    .pin_mosi        = GPIO_Pin_14,
    .pin_sck         = GPIO_Pin_13,
    
    .pin_reset_done  = GPIO_Pin_9,
    .pin_intn        = GPIO_Pin_10,
};

/*
 * Initialise the hardware. This means all GPIOs and SPI for the display
 */
void hw_display_init(void)
{
    display.power_on = 0;
    _display_ready = 0;

    // init display variables
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
    
    //GPIO_InitTypeDef gpio_init_structure;
    GPIO_InitTypeDef gpio_init_disp;
    GPIO_InitTypeDef gpio_init_disp_o;
    
    
    // init the portG display pins (inputs)
    gpio_init_disp.GPIO_Mode = GPIO_Mode_IN;
    gpio_init_disp.GPIO_Pin =  display.pin_reset_done;
    gpio_init_disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_disp.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(display.port_display, &gpio_init_disp);

    // init the portG display pins (inputs)
    gpio_init_disp.GPIO_Mode = GPIO_Mode_IN;
    gpio_init_disp.GPIO_Pin =  display.pin_intn;
    gpio_init_disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_disp.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init_disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOG, &gpio_init_disp);
    
    // init the portG display pins (outputs)
    gpio_init_disp_o.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_disp_o.GPIO_Pin =  display.pin_reset | display.pin_cs;
    gpio_init_disp_o.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_disp_o.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_disp_o.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.port_display, &gpio_init_disp_o);       
        
    // start SPI
    _snowy_display_init_SPI6();   
    _snowy_display_init_dma();

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}


/*
 * We use the Done INTn for the display. This is asserted
 * after every successful command write. I.t. 0x5 to star a frame 
 * will assert done
 */
void _snowy_display_init_intn(void)
{
    // Snowy uses two interrupts for the display
    // Done (G10) signals the drawing is done
    // INTn (G9) I suspect is for device readyness after flash
    //
    // PinSource9 uses IRQ (EXTI9_5_IRQn) and 10 uses (EXTI15_10_IRQn)
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
    
    // Wait for external interrupts when the FPGA is done with a command
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource10);
    
    EXTI_InitStruct.EXTI_Line = EXTI_Line10;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    // display used PinSource10 which is connected to EXTI15_10
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 7;  // must be > 5
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}

/*
 * The display hangs off SPI6. Initialise it
 */
void _snowy_display_init_SPI6(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    SPI_InitTypeDef spi_init_struct;

    // enable clock for used IO pins
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);

    /* configure pins used by SPI6
        * PG13 = SCK
        * PG12 = MISO
        * PG14 = MOSI
        */
    gpio_init_struct.GPIO_Pin = display.pin_miso | display.pin_mosi | display.pin_sck;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.port_display, &gpio_init_struct);

    // connect SPI6 pins to SPI alternate function
    GPIO_PinAFConfig(display.port_display, GPIO_PinSource13, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.port_display, GPIO_PinSource12, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.port_display, GPIO_PinSource14, GPIO_AF_SPI6);


    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);

    // enable peripheral clock
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI6);

    spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
    spi_init_struct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    spi_init_struct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    spi_init_struct.SPI_CPOL = SPI_CPOL_High;        // clock is low when idle
    spi_init_struct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    spi_init_struct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    spi_init_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // SPI frequency is APB2 frequency / 8
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted LSB first
    // pebble additionally has these Set

    SPI_Init(SPI6, &spi_init_struct); 

    SPI_Cmd(SPI6, ENABLE); // enable SPI

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI6);
}

static void _snowy_display_request_clocks()
{
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_DMA2);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI6);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}

static void _snowy_display_release_clocks()
{
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_DMA2);
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI6);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}

/*
 * Initialise DMA for sending frames.
 * DMA is only used for doing a full frame transfer
 * once frame mode select is sent
 */
void _snowy_display_init_dma(void)
{
    NVIC_InitTypeDef nvic_init_struct;
    DMA_InitTypeDef dma_init_struct;
    
    _snowy_display_request_clocks();
    
    // spi6 dma config:
    // SPI6 	DMA2 	DMA Stream 5 	DMA Channel 1 	DMA Stream 6 	DMA Channel 0
    // De-init DMA configuration just to be sure. No Boom
    DMA_DeInit(DMA2_Stream5);
    // Configure DMA controller to manage TX DMA requests
    DMA_Cmd(DMA2_Stream5, DISABLE);
    while (DMA2_Stream5->CR & DMA_SxCR_EN);

    DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_FEIF5|DMA_FLAG_DMEIF5|DMA_FLAG_TEIF5|DMA_FLAG_HTIF5|DMA_FLAG_TCIF5);
    
    DMA_StructInit(&dma_init_struct);
    // set the pointer to the SPI6 DR register
    dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t) &(SPI6->DR);
    dma_init_struct.DMA_Channel = DMA_Channel_1;
    dma_init_struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)0; // set this to bypass assert
    dma_init_struct.DMA_BufferSize = 1;
    dma_init_struct.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    dma_init_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream5, &dma_init_struct);
    
    // enable the tx interrupt.
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
     
    // Enable dma
    SPI_I2S_DMACmd(SPI6, SPI_I2S_DMAReq_Tx, ENABLE);
    
    _snowy_display_release_clocks();
    
    // tell the NVIC to party
    nvic_init_struct.NVIC_IRQChannel = DMA2_Stream5_IRQn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 9;
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
}

/*
 * Start a new Dma transfer
 *
 * Expects clocks to already be running!
 */
void snowy_display_reinit_dma(uint32_t *data, uint32_t length)
{
    DMA_InitTypeDef dma_init_struct;

    // Configure DMA controller to manage TX DMA requests
    DMA_Cmd(DMA2_Stream5, DISABLE);
    while (DMA2_Stream5->CR & DMA_SxCR_EN);

    DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_FEIF5|DMA_FLAG_DMEIF5|DMA_FLAG_TEIF5|DMA_FLAG_HTIF5|DMA_FLAG_TCIF5);
    DMA_StructInit(&dma_init_struct);
    // set the pointer to the SPI6 DR register
    dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t) &SPI6->DR;
    dma_init_struct.DMA_Channel = DMA_Channel_1;
    dma_init_struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)data;
    dma_init_struct.DMA_BufferSize = length;
    dma_init_struct.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_FIFOMode  = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_Priority = DMA_Priority_High;

    DMA_Init(DMA2_Stream5, &dma_init_struct);
    
    // enable the tx interrupt.
    DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
     
    // Enable dma
    SPI_I2S_DMACmd(SPI6, SPI_I2S_DMAReq_Tx, ENABLE);
}

/*
 * DMA2 handler for SPI6
 */
void DMA2_Stream5_IRQHandler()
{
    static uint8_t col_index = 0;
    
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
    {
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

        // check the tx finished
        while (SPI_I2S_GetFlagStatus(SPI6, SPI_I2S_FLAG_TXE) == RESET)
        {
        };
        
        // make sure we are not busy
        while (SPI_I2S_GetFlagStatus(SPI6, SPI_I2S_FLAG_BSY) == SET)
        {
        };

        // if we are finished sending  each column, then reset and stop
        if (col_index < ROW_LENGTH - 1)
        {
            ++col_index;
            // ask for convert and display the next column
            _snowy_display_next_column(col_index);
            return;
        }
                
        // done. We are still in control of the SPI select, so lets let go
        col_index = 0;
        
        _snowy_display_cs(0);
        _display_ready = 1;
        
        /* request_clocks in _snowy_display_start_frame */
        _snowy_display_release_clocks();
        
        display_done_ISR(0);
    }
}

/*
 * Interrupt handler for the INTn Done interrupt on the FPGA
 * When we send a command and it is successfully acked, we get a 
 * GPIO interrupt. We then chain call the ISR handler from here
 * and clear the interrupt
 */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {   
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}

/* When reset goes high, sample the CS input to see what state we should be in
 * if CS is low, expect new FPGA programming to arrive
 * if CS is high, assume we will be using the bootloader configuration
 * 
 *  *NOTE* CS is inverted below
 */
void _snowy_display_cs(uint8_t enabled)
{
    stm32_power_request(STM32_POWER_AHB1, display.clock_display);

    // CS bit is inverted
    if (!enabled)
        GPIO_SetBits(display.port_display, display.pin_cs);
    else
        GPIO_ResetBits(display.port_display, display.pin_cs);

    stm32_power_release(STM32_POWER_AHB1, display.clock_display);
}

/*
 * Request the version from the FPGA in bootloader mode
 */
uint8_t _snowy_display_SPI6_getver(uint8_t data)
{
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until send complete
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until send complete

    return SPI6->DR; // return received data from SPI data register
}

/*
 * Send one byte over the SPI
 */
uint8_t _snowy_display_SPI6_send(uint8_t data)
{
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
    while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}

/*
 * Given a column index, start the conversion of the display data and dma it
 */
void _snowy_display_next_column(uint8_t col_index)
{   
    // set the content
    scanline_convert_column(_column_buffer, display.frame_buffer, col_index);
    _snowy_display_dma_send(_column_buffer, COLUMN_LENGTH);
}

/*
 * Send n bytes over SPI using the DMA engine.
 * This will async run and call the ISR when complete
 */
void _snowy_display_dma_send(uint8_t *data, uint32_t length)
{
    // re-initialise the DMA controller. prep for send
    snowy_display_reinit_dma((uint32_t *)data, length);
    DMA_Cmd(DMA2_Stream5, ENABLE);
    
    return;
}

/*
 * Reset the FPGA. This goes through a convoluted set of steps
 * that basically pound the FPGA into submission. Ya see, sometimes
 * it just doesn't boot nicely. So you have to try a command, no dice?
 * reeeeboot. Over and over until it works.
 * Yay
 */
uint8_t _snowy_display_FPGA_reset(uint8_t mode)
{
    uint16_t k = 0;
    uint8_t g9 = 0;

    _snowy_display_request_clocks();

    // Pull out reset
    _snowy_display_cs(mode);
    delay_ms(1);
    _snowy_display_reset(0);
    _snowy_display_cs(1);

    delay_ms(1);
    
    _snowy_display_reset(1);
    delay_us(1);
    
    // The real pebble at this point will pull reset done once it has reset
    // it also (probably dangerously) "just works" without.
    // it probably isn't the danger zone, but it's the highway
    while(1)
    {
        g9 = GPIO_ReadInputDataBit(display.port_display, display.pin_reset_done);
        
        if (g9 == 0)
        {
            DRV_LOG("FPGA", APP_LOG_LEVEL_DEBUG, "FPGA Reset");
            _snowy_display_release_clocks();
            return 1;
        }
        
        delay_us(100);
            
        k++;
        
        if (k >=1001)
        {
            char *err = "Timed out waiting for reset";
            DRV_LOG("FPGA", APP_LOG_LEVEL_ERROR, err);
            _snowy_display_release_clocks();
            assert(!err);
            return 0;
        }
    }
}

/*
 * Do a hard reset on the display. Make sure to init it!
 */
void _snowy_display_reset(uint8_t enabled)
{
    stm32_power_request(STM32_POWER_AHB1, display.clock_display);

    if (enabled)
        GPIO_SetBits(display.port_display, display.pin_reset);
    else
        GPIO_ResetBits(display.port_display, display.pin_reset);

    stm32_power_release(STM32_POWER_AHB1, display.clock_display);
}

// SPI Command related

/*
 * Shortcut function to address the display device using its
 * Chip Select and wait the "proper" amount of time for engagement
 */
void _snowy_display_SPI_start(void)
{
    _snowy_display_cs(1);
    delay_us(100);
}

/*
 * We're done with this device. Drop the line
 */
void _snowy_display_SPI_end(void)
{
    _snowy_display_cs(0);
}

/*
 * Start to send a frame to the display driver
 * If it says yes, then we can then tell someone to fill the buffer
 */
void _snowy_display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    stm32_power_request(STM32_POWER_AHB1, display.clock_display);
    _snowy_display_request_clocks();

    _snowy_display_cs(1);
    delay_us(10);
    _snowy_display_SPI6_send(DISPLAY_CTYPE_FRAME); // Frame Begin
    _snowy_display_cs(0);
    delay_us(10);

    _display_ready = 0;

    _snowy_display_send_frame();
    /* release_clocks in DMA2_Stream5_IRQHandler */
}

/* 
 * We can fill the framebuffer now. Depending on mode, we will DMA
 * the data directly over to the display
 */
void _snowy_display_send_frame()
{
//     return _snowy_display_send_frame_slow();
    _snowy_display_cs(1);
    delay_us(80);
    // send over DMA
    // we are only going to send one single column at a time
    // the dma engine completion will trigger the next lot of data to go
    _snowy_display_next_column(0);
    // we return immediately and let the system take care of the rest
}

/*
 * Bang the SPI bit by bit
 */
void _snowy_display_send_frame_slow()
{
    _snowy_display_cs(1);
    delay_us(50);
      
    // send via standard SPI
    for(uint8_t x = 0; x < DISPLAY_COLS; x++)
    {
        scanline_convert_column(_column_buffer, display.frame_buffer, x);
        for (uint8_t j = 0; j < DISPLAY_ROWS; j++)
            _snowy_display_SPI6_send(_column_buffer[j]);
    }   
    
    _snowy_display_cs(0);
}

/*
 * Once we reset the FPGA, it takes some time to wake up
 * let's have a tea party and wait for it
 */
uint8_t _snowy_display_wait_FPGA_ready(void)
{
    uint16_t i = 100;
    
    GPIO_InitTypeDef gpio_init_disp;
    
    // init the portG display pins (inputs)
    gpio_init_disp.GPIO_Mode = GPIO_Mode_IN;
    gpio_init_disp.GPIO_Pin =  display.pin_intn;
    gpio_init_disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_disp.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init_disp.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOG, &gpio_init_disp);
    
    while(GPIO_ReadInputDataBit(display.port_display, display.pin_intn) != 0)
    {
        if (!--i)
        {
            char *err = "Timed out waiting for ready";
            DRV_LOG("FPGA", APP_LOG_LEVEL_ERROR, err);
            return 0;
        }        
        delay_us(100);
    }
    DRV_LOG("FPGA", APP_LOG_LEVEL_DEBUG, "FPGA Ready");
    
    return 1;
}

/*
 * Reset the FPGA and send the display engine into full frame mode
 * This will allow raw frame dumps to work
 */
void _snowy_display_full_init(void)
{
    DRV_LOG("Display", APP_LOG_LEVEL_INFO, "Init full driver mode");
    
    _snowy_display_request_clocks();    
    hw_display_on();
    
    if (!_snowy_display_FPGA_reset(0)) // full fat
    {
        _snowy_display_release_clocks();
        assert(!"FGPA Init FAILED!!");
    }
    
    _snowy_display_program_FPGA();
    
    if (_snowy_display_wait_FPGA_ready())
    {
        DRV_LOG("Display", APP_LOG_LEVEL_INFO, "Display is ready");
    }
    
    // enable interrupts now we have the splash up
    _snowy_display_init_intn();   
    _snowy_display_release_clocks();
}

/*
 * Get the source for the display's FPGA, and download it to the device
 */
void _snowy_display_program_FPGA(void)
{
    unsigned char *fpga_blob = &_binary_Resources_FPGA_4_3_snowy_dumped_bin_start;
           
    _snowy_display_cs(1);
    
    // Do this with good ol manual SPI for reliability
    for (uint32_t i = 0; i < (uint32_t)&_binary_Resources_FPGA_4_3_snowy_dumped_bin_size; i++)
    {
        _snowy_display_SPI6_send(*(fpga_blob + i));
    }
    
    _snowy_display_cs(0);
    
    DRV_LOG("Display", APP_LOG_LEVEL_DEBUG, "FPGA bin uploaded");
}


// API (barely) implementation

/*
 * When in bootloader mode, we can command the screen to power on
 * Of little use the end developers now
 */
void hw_display_on()
{
    _snowy_display_request_clocks();

    _snowy_display_SPI_start();
    _snowy_display_SPI6_send(DISPLAY_CTYPE_DISPLAY_ON); // Power on
    _snowy_display_SPI_end();
    
    _snowy_display_release_clocks();
}

/*
 * Start a frame render
 */
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    _snowy_display_start_frame(xoffset, yoffset);
}

uint8_t *hw_display_get_buffer(void)
{
    return display.frame_buffer;
}

uint8_t hw_display_is_ready()
{
    return _display_ready;
}

/*
 * Reset the display
 * Needs work before it works (ahem) after first boot
 * as interrupts are still enabled and get all in the way
 */
void hw_display_reset(void)
{
    hw_display_start();
}

/*
 * Start the display init sequence. from cold start to full framebuffer
 */
void hw_display_start(void)
{
    _snowy_display_full_init();
}


// Util

void delay_us(uint16_t us)
{
    for(int i = 0; i < 22 * us; i++)
            ;;
}

void delay_ms(uint16_t ms)
{
    delay_us(1000 * ms);
}
