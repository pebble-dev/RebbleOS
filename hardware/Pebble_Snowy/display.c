#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stdio.h"
#include "display.h"
#include <stm32f4xx_spi.h>

void display_init_SPI6(void);
uint8_t display_SPI6_send(uint8_t data);


display_t display = {
    .PortDisplay = GPIOG,
    .PinReset = GPIO_Pin_15,
    .PinPower = GPIO_Pin_8, //??
    .PinCs = GPIO_Pin_8, //??
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
    
    .BacklightEnabled = 0,
    .Brightness = 0,
    .PowerOn = 0,
};

void display_init(void) {
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
    display_test(1);
}

void display_backlight(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortBacklight, display.PinBacklight);
    else
        GPIO_ResetBits(display.PortBacklight, display.PinBacklight);
}

void display_vibrate(uint8_t enabled) {
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

void display_power(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinPower);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinPower);
}


void display_reset(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinReset);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinReset);
}

void display_cs(uint8_t enabled) {
    if (enabled)
        GPIO_SetBits(display.PortDisplay, display.PinCs);
    else
        GPIO_ResetBits(display.PortDisplay, display.PinCs);
}


uint8_t display_SPI6_send(uint8_t data) {
    // TODO likely DMA would be better :)
    SPI6->DR = data; // write data to be transmitted to the SPI data register
    while( !(SPI6->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI6->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
    while( SPI6->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
    return SPI6->DR; // return received data from SPI data register
}



void display_test(uint8_t scene) {
    // Recovery screen mode (no FPGA Firmware loaded)
    //PEBBLE_SNOWY_DISPLAY: state change from 0 (PSDISPLAYSTATE_PROGRAMMING) to 1 (PSDISPLAYSTATE_ACCEPTING_CMD)    //frame: 0 - bytes: 0
    //PEBBLE_SNOWY_DISPLAY: DISPLAY POWER CTL changed to 1
    display_power(1);
    //PEBBLE_SNOWY_DISPLAY: CS changed to 1
    display_cs(1);
    //PEBBLE_SNOWY_DISPLAY: RESET changed to 1
    display_reset(1);
    //PEBBLE_SNOWY_DISPLAY: Asserting done interrupt
    //display_sleep(100); // cheat for now until IRQ is kooed up
    //PEBBLE_SNOWY_DISPLAY: Resetting state to accept command
    //PEBBLE_SNOWY_DISPLAY: state change from 1 (PSDISPLAYSTATE_ACCEPTING_CMD) to 1 (PSDISPLAYSTATE_ACCEPTING_CMD) frame: 0 - bytes: 0
    //PEBBLE_SNOWY_DISPLAY: BACKLIGHT ENABLE changed to 0
    //PEBBLE_SNOWY_DISPLAY: CS changed to 0
    display_cs(0);
    /*PEBBLE_SNOWY_DISPLAY: received command 4, deasserting done interrupt
    PEBBLE_SNOWY_DISPLAY: ps_display_execute_current_cmd_set0: cmd: 4 -- cs: CS
    PEBBLE_SNOWY_DISPLAY: state change from 1 (PSDISPLAYSTATE_ACCEPTING_CMD) to 3 (PSDISPLAYSTATE_ACCEPTING_SCENE_BYTE) frame: 0 - bytes: 1
    PEBBLE_SNOWY_DISPLAY: received scene ID: 1
    PEBBLE_SNOWY_DISPLAY: ps_display_execute_current_cmd_set0: cmd: 4 -- cs: CS
    PEBBLE_SNOWY_DISPLAY: Executing command: DRAW_SCENE: 1
    PEBBLE_SNOWY_DISPLAY: Asserting done interrupt
    PEBBLE_SNOWY_DISPLAY: Resetting state to accept command
    PEBBLE_SNOWY_DISPLAY: state change from 3 (PSDISPLAYSTATE_ACCEPTING_SCENE_BYTE) to 1 (PSDISPLAYSTATE_ACCEPTING_CMD) frame: 0 - bytes: 2
    */
    display_SPI6_send(0x04); // set cmdset
    display_SPI6_send(scene); // scene 1
    printf("Setting SPI\n");
    display_cs(1);
    display_cs(0); // actually it looks to be doing a read here.
    //PEBBLE_SNOWY_DISPLAY: CS changed to 1
    //PEBBLE_SNOWY_DISPLAY: CS changed to 0
    //PEBBLE_SNOWY_DISPLAY: received command 3, deasserting done interrupt
    //PEBBLE_SNOWY_DISPLAY: ps_display_execute_current_cmd_set0: cmd: 3 -- cs: CS
    //PEBBLE_SNOWY_DISPLAY: Executing command: DISPLAY_ON
    //PEBBLE_SNOWY_DISPLAY: Asserting done interrupt
    //PEBBLE_SNOWY_DISPLAY: Resetting state to accept command
    //PEBBLE_SNOWY_DISPLAY: state change from 1 (PSDISPLAYSTATE_ACCEPTING_CMD) to 1 (PSDISPLAYSTATE_ACCEPTING_CMD) frame: 0 - bytes: 3

    //PEBBLE_SNOWY_DISPLAY: CS changed to 1
    //PEBBLE_SNOWY_DISPLAY: Backlight level changed to 0
    //PEBBLE_SNOWY_DISPLAY: Backlight level changed to 0
    display_cs(1);
    display_SPI6_send(0x03); // power on
}
