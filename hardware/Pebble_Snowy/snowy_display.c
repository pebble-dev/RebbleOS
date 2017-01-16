#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "snowy_display.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>

// pointer to the place in flash where the FPGA image resides
extern unsigned char _binary_PebbleImages_FPGA_4_3_snowy_dumped_bin_start;
extern unsigned char _binary_PebbleImages_FPGA_4_3_snowy_dumped_bin_size;

display_t display = {
    .PortDisplay    = GPIOG,
    .PinReset       = GPIO_Pin_15,
    .PinCs          = GPIO_Pin_8, 
    .PinBacklight   = GPIO_Pin_14,
    .PortBacklight  = GPIOB,
    .PinMiso        = GPIO_Pin_12,
    .PinMosi        = GPIO_Pin_14,
    .PinSck         = GPIO_Pin_13,
    
    .PinResetDone   = GPIO_Pin_9,
    .PinIntn        = GPIO_Pin_10,
    
    .NumRows        = 168,
    .NumCols        = 144,
    .NumBorderRows  = 2,
    .NumBorderCols  = 2,
};


void hw_display_init(void)
{   
    // init display variables
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        
    printf("Display Init\n");
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure_Disp;
    GPIO_InitTypeDef GPIO_InitStructure_Disp_o;
    
    // these are set. not sure why, but will set them for completeness for now
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    
    
    // init the backlight (pins 2 & 3 are for something. not wure yet)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // bootloader puls port D2 + D4 high
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    
    // init the portG display pins (inputs)
    GPIO_InitStructure_Disp.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure_Disp.GPIO_Pin =  display.PinResetDone;
    GPIO_InitStructure_Disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp);

    // init the portG display pins (inputs)
    GPIO_InitStructure_Disp.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure_Disp.GPIO_Pin =  display.PinIntn;
    GPIO_InitStructure_Disp.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure_Disp.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure_Disp.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOG, &GPIO_InitStructure_Disp);
    
    // init the portG display pins (outputs)
    GPIO_InitStructure_Disp_o.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Disp_o.GPIO_Pin =  display.PinReset; // | display.PinPower;
    GPIO_InitStructure_Disp_o.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp_o.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure_Disp_o.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp_o);
    
    
    // start SPI
    snowy_display_init_SPI6();
    
    
//     // bootloader does this too. Why??
//     GPIO_SetBits(GPIOD, GPIO_Pin_2);
//     GPIO_SetBits(GPIOD, GPIO_Pin_4);
//     
//     GPIO_SetBits(GPIOF, GPIO_Pin_3);
//     GPIO_SetBits(GPIOF, GPIO_Pin_2);

}

void hw_display_reset(void)
{
    hw_display_start();
}

void hw_display_start(void)
{
    // begin the init
    snowy_display_splash(2);
    delay_us(100);
    snowy_display_full_init();
}

void snowy_display_init_intn(void)
{
    // Snowy uses two interrupts for the display
    // Done (G10) signals the drawing is done
    // INTn (G9) I suspect is for device readyness after flash
    //
    // PinSource9 uses IRQ (EXTI9_5_IRQn) and 10 uses (EXTI15_10_IRQn)
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    // actually, I don't much care about this. It's polled on init now anyway
//     SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource9);
//     
//     EXTI_InitStruct.EXTI_Line = EXTI_Line9;
//     EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//     EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
//     EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
//     EXTI_Init(&EXTI_InitStruct);
//  
//     NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
//     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 7;
//     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
//     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStruct);
//        
    
    // now do INTn
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
}


void snowy_display_init_timer(uint16_t pwmValue)
{
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TIM_OCInitTypeDef TIM_OCStruct;

    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Pebble Time has backlight control driven by TIM12
    // It is set to PWM mode 2 and will count up to n
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);
    
    /* Set pins */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    TIM_BaseStruct.TIM_Prescaler = 0;
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_Period = 9999; // 10khz calculated by clock dividers
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM12, &TIM_BaseStruct);
    
    // now the OC timer
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
    
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;  // set on compare
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    
    TIM_OCStruct.TIM_Pulse = pwmValue;
    TIM_OC1Init(TIM12, &TIM_OCStruct);
    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);
    
    TIM_Cmd(TIM12, ENABLE);
    TIM_CtrlPWMOutputs(TIM12, ENABLE);
}


// this function initializes the SPI6 peripheral
void snowy_display_init_SPI6(void)
{	
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    // enable clock for used IO pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    /* configure pins used by SPI6
        * PG13 = SCK
        * PG12 = MISO
        * PG14 = MOSI
        */
    GPIO_InitStruct.GPIO_Pin = display.PinMiso | display.PinMosi | display.PinSck;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.PortDisplay, &GPIO_InitStruct);

    // connect SPI6 pins to SPI alternate function
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource13, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource12, GPIO_AF_SPI6);
    GPIO_PinAFConfig(display.PortDisplay, GPIO_PinSource14, GPIO_AF_SPI6);

    
    GPIO_InitStruct.GPIO_Pin = display.PinCs;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.PortDisplay, &GPIO_InitStruct);

    // enable peripheral clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6, ENABLE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;        // clock is low when idle
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 2
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted LSB first
    // pebble additionally has these Set

    SPI_Init(SPI6, &SPI_InitStruct); 

    SPI_Cmd(SPI6, ENABLE); // enable SPI
}


// Interrupt handler for Command Complete from the FPGA
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {   
        display_done_ISR(1);
       
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}

// When reset goes high, sample the CS input to see what state we should be in
// if CS is low, expect new FPGA programming to arrive
// if CS is high, assume we will be using the bootloader configuration
void snowy_display_cs(uint8_t enabled)
{
    // CS bit is inverted
    if (!enabled)
        GPIO_SetBits(display.PortDisplay, display.PinCs);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinCs);
}

uint8_t snowy_display_SPI6_getver(uint8_t data)
{
    // TODO likely DMA would be better :)
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until send complete
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until send complete
    //while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}

uint8_t snowy_display_SPI6_send(uint8_t data)
{
    // TODO likely DMA would be better :)
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
    while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}

uint8_t snowy_display_FPGA_reset(uint8_t mode)
{
    uint16_t k = 0;
    uint8_t g9 = 0;
    

    snowy_display_cs(mode);
//    snowy_display_cs(1);
    snowy_display_reset(0);
    delay_ns(1);
    snowy_display_reset(1);
    
    // don't wait when we are not in bootloader mode
    // qemu doesn't like this
//    if (mode == 1)
        return 1;
    
    while(1)
    {
        g9 = GPIO_ReadInputDataBit(display.PortDisplay, display.PinResetDone);
        
        if (g9 > 0)
        {
            printf("FPGA Was reset\n");
            return 1;
        }
        
        delay_us(100);
            
        k++;
        
        if (k >=1001)
        {
            printf("timed out waiting for reset\n");
            //display_vibrate(1);
            vibrate_enable(1);
            return 0;
        }
    }
}

// Pull the reset pin
void snowy_display_reset(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinReset);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinReset);
}

// SPI Command related

void snowy_display_SPI_start(void)
{
    snowy_display_cs(1);
    delay_us(100);
}

void snowy_display_SPI_end(void)
{
    snowy_display_cs(0);
}

// Draw a scene
void snowy_display_drawscene(uint8_t scene)
{
    snowy_display_SPI_start();
    snowy_display_SPI6_send(DISPLAY_CTYPE_SCENE); // set cmdset (select scene)
    snowy_display_SPI6_send(scene); // set the scene id
    snowy_display_SPI_end();
}

void snowy_display_start_frame(void)
{
    snowy_display_cs(1);
    snowy_display_SPI6_send(DISPLAY_CTYPE_FRAME); // Frame Begin
    snowy_display_cs(0);
}

void snowy_display_send_frame()
{
    // pull CS low here becuase we are ususally called from an interrupt and we don't
    // like race conditions

    delay_us(100);
    snowy_display_cs(1);
    delay_us(100);
    for(uint16_t i = 0; i < 24192; i++)
    {
        snowy_display_SPI6_send(display.DisplayBuffer[i]);
    }
    printf("End Frame\n");
    delay_us(100);
    snowy_display_cs(0);
}


uint8_t snowy_display_wait_FPGA_ready(void)
{
    uint16_t i = 1000;
    
    while(GPIO_ReadInputDataBit(display.PortDisplay, display.PinIntn) != 0)
    {
        if (!--i)
        {
            printf("timed out waiting for ready\n");
            return 0;
        }        
        delay_us(100);
    }
    printf("FPGA Ready\n");
    
    return 1;
}


void snowy_display_splash(uint8_t scene)
{  
    int g10 = 0;
    int g9 = 0;
    
    if (!snowy_display_FPGA_reset(0)) // mode bootloader
    {
        return;
    }
  
        
    //get the version This is done in bootloader at least once
//     display_cs(1);
//     for(int i = 0; i < 1000; i++)
//         ;;
//     display_SPI6_getver(0);
//     
//     display_cs(0);
//     
    
    
    // once a scene is selected, we need to wait for the FPGA to respond.
    // it seems like sometimes it doesn't behave, so we reset it until it does 
    // I have one device that requires 5 cycles before it is alive!
    // known issue apparently (Katharine Berry confirmed)
    for (uint8_t i = 0; i < 10; i++)
    {
        delay_ns(1);
        snowy_display_cs(1);
        delay_us(100);
        snowy_display_SPI6_send(4); // Scene select
        snowy_display_SPI6_send(scene); // Select scene
        snowy_display_cs(0);
        
        IWDG_ReloadCounter();
        delay_us(100);
        
        if (snowy_display_wait_FPGA_ready())
        {
            snowy_display_cs(1);
            delay_us(100);
            snowy_display_SPI6_send(3); // power on
            snowy_display_cs(0);

            printf("Display Init Complete\n");
            
            break;
        }
        
        snowy_display_FPGA_reset(0);
    }
    
    return;                
}

// send the FPGA firmware and send the first frame
void snowy_display_full_init(void)
{
    printf("Going full fat\n");
    
    if (!snowy_display_FPGA_reset(1)) // full fat
    {
        return;
    }
    
    display_checkerboard(display.DisplayBuffer, 0);
    
    snowy_display_program_FPGA();
    delay_us(100);
    snowy_display_start_frame();
    delay_us(10);
    snowy_display_send_frame();
    
    IWDG_ReloadCounter();
    
    // enable interrupts now we have the splash up
    snowy_display_init_intn();
    return;
}


void snowy_display_program_FPGA(void)
{
    unsigned char *fpgaBlob = &_binary_PebbleImages_FPGA_4_3_snowy_dumped_bin_start;
           
    // enter programming mode
    snowy_display_cs(1);
    
    for (uint32_t i = 0; i < (uint32_t)&_binary_PebbleImages_FPGA_4_3_snowy_dumped_bin_size; i++)
    {
        snowy_display_SPI6_send(*(fpgaBlob + i));
    }
    snowy_display_cs(0);
    
    printf("Sent FPGA dump\n");
}


// API (barely) implementation

// command the screen on
void hw_display_on()
{
    snowy_display_SPI_start();
    snowy_display_SPI6_send(DISPLAY_CTYPE_DISPLAY_ON); // Power on
    snowy_display_SPI_end();
}

void hw_display_start_frame(void)
{
    snowy_display_start_frame();
}

void hw_display_send_frame(void)
{
    snowy_display_send_frame();
}


void hw_backlight_init(void)
{
    hw_backlight_set(8499);
}

void hw_backlight_set(uint16_t val)
{
    printf("Backlight: Set\n");
    // It seems a little heavy handed reinitialising the whole
    // timer with the new param, but it seems like pebble
    // hardware does this already
    snowy_display_init_timer(val);
}



// Util

void delay_us(uint16_t us)
{
    for(int i = 0; i < 22 * us; i++)
            ;;
}

void delay_ns(uint16_t ns)
{
    delay_us(1000 * ns);
}
