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
#include "stm32_spi.h"
#include "platform_config.h"
#include "rebble_memory.h"
#include "resource.h"

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
static void _spi_tx_done(void);

// pointer to the place in flash where the FPGA image resides
// extern unsigned char fpga_address; // _binary_Resources_FPGA_4_3_snowy_dumped_bin_start;
// extern unsigned char fpga_size; //binary_Resources_FPGA_4_3_snowy_dumped_bin_size;

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
CCRAM display_t display = {
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


static const stm32_spi_config_t _spi6_config = {
    .spi               = SPI6,
    .spi_periph_bus    = STM32_POWER_APB2,
    .gpio_pin_miso_num = 12,
    .gpio_pin_mosi_num = 14,
    .gpio_pin_sck_num  = 13,
    .gpio_ptr          = GPIOG,
    .gpio_clock        = RCC_AHB1Periph_GPIOG,
    .spi_clock         = RCC_APB2Periph_SPI6,
    .af                = GPIO_AF_SPI6,
    .crc_poly          = 7,
    .line_polarity     = SPI_CPOL_High,
    .txrx_dir          = STM32_SPI_DIR_RXTX
};

static const stm32_dma_t _spi6_dma = {
    .dma_clock            = RCC_AHB1Periph_DMA2,
    .dma_tx_stream        = DMA2_Stream5,
    .dma_rx_stream        = DMA2_Stream6,
    .dma_tx_channel       = DMA_Channel_1,
    .dma_rx_channel       = DMA_Channel_0,
    .dma_irq_tx_pri       = 9,
    .dma_irq_rx_pri       = 9,
    .dma_irq_tx_channel   = DMA2_Stream5_IRQn,
    .dma_irq_rx_channel   = DMA2_Stream6_IRQn,
    .dma_tx_channel_flags = STM32_DMA_MK_FLAGS(5),
    .dma_rx_channel_flags = STM32_DMA_MK_FLAGS(6),
    .dma_tx_irq_flag      = DMA_IT_TCIF5,
    .dma_rx_irq_flag      = DMA_IT_TCIF6,
};

static stm32_spi_t _spi6 = {
    &_spi6_config,
    &_spi6_dma, /* dma */
};

void _spi_tx_done(void);

// SPI6 	DMA2 	DMA Stream 5 	DMA Channel 1 	DMA Stream 6 	DMA Channel 0
STM32_SPI_MK_TX_IRQ_HANDLER(&_spi6, 2, 5, _spi_tx_done)

/*
 * Initialise the hardware. This means all GPIOs and SPI for the display
 */
void hw_display_init(void)
{
//     display.power_on = 0;
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
//     _snowy_display_init_dma();

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
    stm32_spi_init_device(&_spi6);
}

static void _snowy_display_request_clocks()
{
//     stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_DMA2);
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI6);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}

static void _snowy_display_release_clocks()
{
//     stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_DMA2);
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI6);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOG);
}

/*
 * Start a new Dma transfer
 *
 * Expects clocks to already be running!
 */
void snowy_display_reinit_dma(uint32_t *data, uint32_t length)
{
//     _snowy_display_request_clocks();
    stm32_spi_send_dma(&_spi6, data, length);
}



/*
 * DMA2 handler for SPI6
 */
static void _spi_tx_done(void)
{
    static uint8_t col_index = 0;

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

/*
 * Interrupt handler for the INTn Done interrupt on the FPGA
 * When we send a command and it is successfully acked, we get a 
 * GPIO interrupt. We then chain call the ISR handler from here
 * and clear the interrupt
 */
/*void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {   
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}*/

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
    scanline_convert(_column_buffer, display.frame_buffer, col_index);
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
    delay_large(1);
    _snowy_display_reset(0);
    _snowy_display_cs(1);

    delay_large(1);
    
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
    _snowy_display_request_clocks();

    _snowy_display_cs(1);
    delay_us(10);
    _snowy_display_SPI6_send(DISPLAY_CTYPE_FRAME); // Frame Begin
    _snowy_display_cs(0);
    delay_us(250);

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
    _snowy_display_request_clocks();
    
    _snowy_display_cs(1);
    delay_us(50);
    _snowy_display_SPI6_send(DISPLAY_CTYPE_FRAME); // Frame Begin
    _snowy_display_cs(0);
    delay_us(10);
    
    _snowy_display_cs(1);
    
    // send via standard SPI
    for(uint8_t x = 0; x < DISPLAY_COLS; x++)
    {
        scanline_convert(_column_buffer, display.frame_buffer, x);
        for (uint8_t j = 0; j < DISPLAY_ROWS; j++)
            _snowy_display_SPI6_send(_column_buffer[j]);
    }   
    
    _snowy_display_cs(0);
   _snowy_display_release_clocks();
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
/*    
    // get the splashscreen resource handle and read it directly into the framebuffer
    ResHandle resource_handle = resource_get_handle_system(SPLASH_RESOURCE_ID);
    hw_flash_read_bytes(REGION_RES_START + RES_START + resource_handle.offset, display.frame_buffer, resource_handle.size);
    // send raw splashscreen image to display
    _snowy_display_send_frame_slow();
*/

    // enable interrupts now we have the splash up
    // Interrupt handler is in with bluetooth :/
    _snowy_display_init_intn();   
    _snowy_display_release_clocks();
}

/*
 * Get the source for the display's FPGA, and download it to the device
 */
void _snowy_display_program_FPGA(void)
{
    unsigned char *fpga_blob = DISPLAY_FPGA_ADDR;
           
    _snowy_display_cs(1);
    
    // Do this with good ol manual SPI for reliability
    for (uint32_t i = 0; i < (uint32_t)DISPLAY_FPGA_SIZE; i++)
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

void delay_large(uint16_t ms)
{
    delay_us(1000 * ms);
}
