#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "task.h"
#include "semphr.h"
#include <stm32f4xx_spi.h>

void vDisplayTask(void *pvParameters);
void display_init_SPI6(void);
void display_init_intn(void);
uint8_t display_SPI6_send(uint8_t data);
uint8_t display_SPI6_send_word(uint16_t data);
void display_done_ISR(uint8_t cmd);

static TaskHandle_t xDisplayTask;
static xQueueHandle xQueue;
static xQueueHandle xIntQueue;

void display_on();
void display_power(uint8_t enabled);
void display_reset(uint8_t enabled);
void display_init_FPGA(uint8_t drawMode);
void display_drawscene(uint8_t scene);
void display_start_frame(char *frameData);

display_t display = {
    .PortDisplay = GPIOG,
    .PinReset = GPIO_Pin_15,
    .PinPower = GPIO_Pin_8, // cs and power? I don't think this one is used tbh
    .PinCs = GPIO_Pin_8, 
    .PinBacklight = GPIO_Pin_14,
    .PortBacklight = GPIOB,
    .PinVibrate = GPIO_Pin_4,
    .PortVibrate = GPIOF,
    .PinMiso = GPIO_Pin_12,
    .PinMosi = GPIO_Pin_14,
    .PinSck = GPIO_Pin_13,
    
    .PinDone = GPIO_Pin_9,
    .PinIntn = GPIO_Pin_10,
    
    .num_rows = 172,
    .num_cols = 148,
    .num_border_rows = 2,
    .num_border_cols = 2,
    .row_major = 0,
    .row_inverted = 0,
    .col_inverted = 0,
};

void display_init(void)
{
    // init display variables
    
    display.BacklightEnabled = 0;
    display.Brightness = 0;
    display.PowerOn = 0;
    display.DisplayState = 0;
    
    printf("Display Init\n");
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure_Vibr;
    GPIO_InitTypeDef GPIO_InitStructure_Disp;
    GPIO_InitTypeDef GPIO_InitStructure_Disp_o;
    
    // init the backlight
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = display.PinBacklight;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortBacklight, &GPIO_InitStructure);
    
    // Timer 12 is also used to generate the brightness
    
    
    // init the vibrator
    GPIO_InitStructure_Vibr.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Vibr.GPIO_Pin = display.PinVibrate;
    GPIO_InitStructure_Vibr.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Vibr.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Vibr.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortVibrate, &GPIO_InitStructure_Vibr);
    
    // init the portG display pins (inputs)
    GPIO_InitStructure_Disp.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure_Disp.GPIO_Pin =  display.PinDone | display.PinIntn;
    GPIO_InitStructure_Disp.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure_Disp.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp);
    
    // init the portG display pins (outputs)
    GPIO_InitStructure_Disp_o.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure_Disp_o.GPIO_Pin =  display.PinReset | display.PinPower;
    GPIO_InitStructure_Disp_o.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure_Disp_o.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure_Disp_o.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(display.PortDisplay, &GPIO_InitStructure_Disp_o);
    
    // start SPI
    display_init_SPI6();
    //display_test(1);
    
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL, &xDisplayTask); 
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
    xIntQueue = xQueueCreate( 4, sizeof(uint8_t) );
    
    printf("Display Task Created!\n");
    display_init_intn();
    
    // turn on the LCD draw
    display.DisplayMode = DISPLAY_MODE_BOOTLOADER;
    display_cmd(DISPLAY_CMD_INIT, NULL);

}


void display_init_intn(void)
{
    // Snowy uses two interrupts for the display
    // Done (G9) signals the drawing is done
    // INTn (G10) I suspect is for device readyness after flash
    //
    // PinSource9 uses IRQ (EXTI9_5_IRQn) and 10 uses (EXTI15_10_IRQn)
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource9);
    
    /* Button is connected to EXTI_Linen */
    EXTI_InitStruct.EXTI_Line = EXTI_Line9;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 13;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
    
    
    // now do INTn
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource10);
    
    /* Button is connected to EXTI_Linen */
    EXTI_InitStruct.EXTI_Line = EXTI_Line10;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 13;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
}

/* Set interrupt handlers */
void EXTI9_5_IRQHandler(void)
{
printf("Reset Complete\n");
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {       
        uint8_t cmd = display.DisplayState;
        display_done_ISR(cmd);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}


/* Set interrupt handlers */
void EXTI15_10_IRQHandler(void)
{
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {   
        uint8_t cmd = display.DisplayState;
        display_done_ISR(cmd);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}

void display_done_ISR(uint8_t cmd) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
printf("Done ISR\n");
    /* Notify the task that the transmission is complete. */
    //vTaskNotifyGiveFromISR( xDisplayTask, &xHigherPriorityTaskWoken );

    xQueueSendToBackFromISR(xIntQueue, &cmd, 0);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}




void display_backlight(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortBacklight, display.PinBacklight);
    else
        GPIO_ResetBits(display.PortBacklight, display.PinBacklight);
}

void display_vibrate(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortVibrate, display.PinVibrate);
    else
        GPIO_ResetBits(display.PortVibrate, display.PinVibrate);
}


// this function initializes the SPI6 peripheral
void display_init_SPI6(void){	
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    // enable clock for used IO pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    /* configure pins used by SPI6
        * PG13 = SCK
        * PG12 = MISO
        * PG14 = MOSI
        */
    GPIO_InitStruct.GPIO_Pin = display.PinMiso | display.PinMosi | display.PinReset;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(display.PortDisplay, &GPIO_InitStruct);

    // connect SPI6 pins to SPI alternate function
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_SPI6);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF_SPI6);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_SPI6);

    
    /* Configure the chip select pin
        in this case we will use PE7 */
    /*GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIOE->BSRRL |= GPIO_Pin_7; // set PE7 high
*/
    // enable peripheral clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6, ENABLE);

    /* configure SPI1 in Mode 0 
        * CPOL = 0 --> clock is low when idle
        * CPHA = 0 --> data is sampled at the first edge
        */
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 4
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
    SPI_Init(SPI6, &SPI_InitStruct); 

    SPI_Cmd(SPI6, ENABLE); // enable SPI1
}

// When reset goes high, sample the CS input to see what state we should be in
// if CS is low, expect new FPGA programming to arrive
// if CS is high, assume we will be using the bootloader configuration
void display_cs(uint8_t enabled)
{
    // CS bit is inverted
    if (!enabled)
        GPIO_SetBits(display.PortDisplay, display.PinCs);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinCs);
}


uint8_t display_SPI6_send(uint8_t data)
{
    // TODO likely DMA would be better :)
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
    while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}

uint8_t display_SPI6_send_word(uint16_t data)
{
    uint8_t high = (data >> 4) & 0x0F;
    uint8_t low = data & 0x0F;

    display_SPI6_send(low);
    display_SPI6_send(high);
    
    return SPI6->DR;
}

// display state machine based stuff
void display_init_FPGA(uint8_t drawMode)
{
    printf("Init FPGA\n");
    display.DisplayMode = drawMode;
    // Enable power to the FPGA
    display_power(1);
    // Turn on SPI Chip Select
    // if drawmode is 0 then it is bootloader
    // 1 is real display
    display_cs(drawMode);

    // Reset the current command set
    display_reset(0);
    display_reset(1);

    // now we wait for the done IRQ
}

// Enable the power pin
void display_power(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinPower);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinPower);
}

// Pull the reset pin
void display_reset(uint8_t enabled)
{
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinReset);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinReset);
}

void display_drawscene(uint8_t scene)
{
    printf("Select Scene\n");
    display_cs(1);
    display_SPI6_send(0x04); // set cmdset (select scene)
    display_SPI6_send(scene); // scene 1

    display_cs(0);
}


// command the screen on
void display_on()
{
    display_cs(1);
    display_SPI6_send(0x03); // Power on
    printf("Power On\n");
    display_cs(0);
}

void display_start_frame(char *frameData)
{
    display_cs(1);
    display_SPI6_send(0x05); // Frame Begin
    printf("Frame\n");
//display_SPI6_send(0x3F);
    int count = 0;
    int gridx = 0;
    int gridy = 0;
    
    for (int x = 0; x < 144; x++) {
        gridy = 0;
        if ((x % 21) == 0)
            gridx++;
        for (int y = 0; y < 168; y++) {
            if ((y % 21) == 0)
                gridy++;
            
            if (gridy%2 == 0)
            {
                if (gridx%2 == 0)    
                    display_SPI6_send(4);
                else
                    display_SPI6_send(0);
            }
            else
            {
                if (gridx%2 == 0)    
                    display_SPI6_send(0);
                else
                    display_SPI6_send(4);
            }
        }
    }
    //display_SPI6_send(0x3F);
/*
    int j = 0;
    for(int i = 0; i < 24193; i++)
    {
      
        if (j < 50)
        {
            display_SPI6_send(0x30);
            j++;
        }
        else if (
        {
            display_SPI6_send(0xFC);
            j = 0;
        }
    }*/
    printf("End Frame\n");
    display_cs(0);
}


void display_cmd(uint8_t cmd, char *data)
{
    xQueueSendToBack(xQueue, &cmd, 0);
}

void display_program_FPGA(void)
{
    printf("Sending FPGA dump\n");
    
    // OMG no
    static const char fakedump[32200] = {
  0xFF, 0x00, 0x4C, 0x61, 0x74, 0x74, 0x69, 0x63, 0x65, 0x00, 0x69, 0x43, 0x45, 0x63,      //pG..Lattice.iCEc
  0x75, 0x62, 0x65, 0x32, 0x20, 0x32, 0x30, 0x31, 0x34, 0x2E, 0x30, 0x38, 0x2E, 0x32, 0x36, 0x37,      //ube2 2014.08.267
  0x32, 0x33, 0x00, 0x50, 0x61, 0x72, 0x74, 0x3A, 0x20, 0x69, 0x43, 0x45, 0x34, 0x30, 0x4C, 0x50,      //23.Part: iCE40LP
  0x31, 0x4B, 0x2D, 0x43, 0x4D, 0x33, 0x36, 0x00, 0x44, 0x61, 0x74, 0x65, 0x3A, 0x20, 0x44, 0x65,      //1K-CM36.Date: De
  0x63, 0x20, 0x31, 0x30, 0x20, 0x32, 0x30, 0x31, 0x34, 0x20, 0x30, 0x38, 0x3A, 0x33, 0x30, 0x3A,      //c 10 2014 08:30:
  0x00, 0xFF, 0x31, 0x38, 0x00, 0x7E, 0xAA, 0x99, 0x7E, 0x51, 0x00, 0x01, 0x05, 0x92, 0x00, 0x20,      //..18.~..~Q.....
  0x62, 0x01, 0x4B, 0x72, 0x00, 0x90, 0x82, 0x00, 0x00, 0x11, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00 };

    // enter programming mode
    display_cs(1);
    
    int len = strlen(fakedump);
    for (uint32_t i = 0; i < len; i++)
    {
        display_SPI6_send(fakedump[i]);
    }
    display_cs(0);
}

void vDisplayTask(void *pvParameters)
{
    uint8_t data;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    display.DisplayState = DISPLAY_CMD_IDLE;
    
    while(1)
    {
        // commands to be exectuted are send to this queue and processed
        // one at a time
        if (xQueueReceive(xQueue, &data, xMaxBlockTime))
        {
            printf("CMD\n");
            display.DisplayState = data;
            switch(data)
            {
                case DISPLAY_CMD_DISPLAY_ON:
                    display_on();
                    break;
                case DISPLAY_CMD_RESET:
                    display_reset(1);                    
                    break;
                case DISPLAY_CMD_IDLE:
                    display.DisplayState = DISPLAY_CMD_IDLE;
                    break;
                case DISPLAY_CMD_INIT:
                     display_init_FPGA(display.DisplayMode);
                    break;
                case DISPLAY_CMD_INITF:
                     display.DisplayMode = DISPLAY_MODE_FULLFAT;
                     display_init_FPGA(DISPLAY_MODE_FULLFAT);
                    break;
                case DISPLAY_CMD_FLASH:
                    printf("Programming FPGA...\n");
                    display_program_FPGA();
                    break;
                case DISPLAY_CMD_BEGIN:
                    display_drawscene(2);
                    //display_on();
                    break;
                case DISPLAY_CMD_DRAW:
                    display_start_frame(NULL);
                    break;
            }
            
            // once in full fat mode, we dont get asserts
            if (display.DisplayMode == DISPLAY_MODE_FULLFAT && 
                display.DisplayMode != DISPLAY_CMD_DRAW)
                continue;
            
            // This is prett terrible as far as usage goes, but in order to satisfy qemu
            // acking the "done" interrupt is another queue. The ints then stuff the queue on completion
            // in order to syncronise the commands
            // Don't know about the real hardware, but the int is triggering before we can 
            // set the semaphore. Setting up NVIC priorities could solve that later
            if (xQueueReceive(xIntQueue, &data, xMaxBlockTime))
            {           
                if (display.DisplayState == DISPLAY_CMD_INIT)
                {
                    // as a bonus followup of the init, we are turning on the full fat interface
                    display_cmd(DISPLAY_CMD_BEGIN, NULL);
                    display_cmd(DISPLAY_CMD_DISPLAY_ON, NULL);
                    display_cmd(DISPLAY_CMD_INITF, NULL);                   
                    display_cmd(DISPLAY_CMD_FLASH, NULL);
                    display_cmd(DISPLAY_CMD_DRAW, NULL);
                    //display_cmd(DISPLAY_CMD_DRAW, NULL);
                }
                printf("ISR Idled\n");
                
                display.DisplayState = DISPLAY_CMD_IDLE;
            }
            else
            {
                // The call to ulTaskNotifyTake() timed out
                printf("no ISR Responses\n");
                display.DisplayState = DISPLAY_CMD_IDLE;
            }
        }
        else
        {
            // nothing emerged from the buffer
            printf("Display heartbeat\n");
            // do one second(ish) maint tasks
            
        }        
    }
}
