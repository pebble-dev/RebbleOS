/* snowy_bluetooth.c
 * Hardware driver for the cc256x chipset
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "log.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_tim.h>
#include "stm32_power.h"
#include "btstack_rebble.h"
#include "snowy_bluetooth.h"
#include "stm32_usart.h"

#define BT_SHUTD        GPIO_Pin_12

static void _bt_reset_hci_dma(void);
static void _bluetooth_dma_init(void);
static void _usart1_init(uint32_t baud);

static const stm32_usart_config_t _usart1_config = {
    .usart                = USART1,
    .flow_control_enabled = FLOW_CONTROL_ENABLED,
    .usart_periph_bus     = STM32_POWER_APB2,
    .gpio_pin_tx_num      = 9,
    .gpio_pin_rx_num      = 10,
    .gpio_pin_rts_num     = 12,
    .gpio_pin_cts_num     = 11,
    .gpio_ptr             = GPIOA,
    .gpio_clock           = RCC_AHB1Periph_GPIOA,
    .usart_clock          = RCC_APB2Periph_USART1,
    .af                   = GPIO_AF_USART1,
};

/* dma tx: dma stream 7 chan 4, rx: stream 2 chan 4. 
 * irq tx: 6, rx: 8 */
static const stm32_dma_t _usart1_dma = STM32_DMA_MK_INIT(RCC_AHB1Periph_DMA2, 2, 7, 2, 4, 4, 6, 8); 

static stm32_usart_t _usart1 = {
    &_usart1_config,
    &_usart1_dma, /* dma */
    115200 /* initial slow handshake */
};

/* DMA 2 stream 7. IRQ handlers */
STM32_USART_MK_TX_IRQ_HANDLER(&_usart1, 2, 7, bt_stack_tx_done)
STM32_USART_MK_RX_IRQ_HANDLER(&_usart1, 2, 2, bt_stack_rx_done)

uint8_t hw_bluetooth_init(void)
{
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    /* B12: The Bluetooth nSHUTD pin "shutdown"
     * Data is driver user USART1
     * The 32.768Khz "slow clock" is taken from the
     * "Low Speed External" (LSE) oscillator
     * and redirected out of MCO1 pin on the STM
     */
    
    GPIO_InitTypeDef gpio_init_bt;
    
    /* nSHUTD on B12 */
    gpio_init_bt.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_bt.GPIO_Pin =  BT_SHUTD;
    gpio_init_bt.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_bt.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_bt.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &gpio_init_bt);
    
    /* Not entirely sure what this is for, but might help? */
    gpio_init_bt.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_bt.GPIO_Pin =  GPIO_Pin_4;
    gpio_init_bt.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_bt.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_bt.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOA, &gpio_init_bt);
    
    /* Set MCO as an AF output */
    gpio_init_bt.GPIO_Mode = GPIO_Mode_AF;
    gpio_init_bt.GPIO_Pin =  GPIO_Pin_8;
    gpio_init_bt.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_bt.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_bt.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &gpio_init_bt);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
   
    /* configure DMA and set initial speed */
    stm32_usart_init_device(&_usart1);
    
    /* I'm going to request the usart stay on for now pending bluetooth */
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_USART1);

    hw_bluetooth_clock_on();

    /*
     * Initialise the device stack
     */
    bt_device_init();
    
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "Done...\n");

    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);     
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    
    return 0;
}

/* Enable the external 32.768khz clock for the bluetooth.
 * The clock is connected to the Low speed External LSE input
 * It's used to drive RTC and also the bluetooth module.
 * We are going to pass the clock out through MCO1 pin
 */
void hw_bluetooth_clock_on(void)
{
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: Powering up external clock...");
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_USART1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_PWR);

    /* I see this in traces. What am I? */
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    
    /* allow RTC access so we can play with LSE redirection */
    PWR_BackupAccessCmd(ENABLE); 
    
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT set MCO1 32khz");
    /* Turn on the MCO1 clock and pump 32khz to bluetooth */
    RCC_LSEConfig(RCC_LSE_ON);

    /* knock knock */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "LSE Ready");
    
    /* Enable power to the MCO Osc out pin */
    RCC_MCO1Config(RCC_MCO1Source_LSE, RCC_MCO1Div_1);
        
    /* datasheet says no more than 2ms for clock stabilisation. 
     * Lets go for more */
    delay_ms(5);
    IWDG_ReloadCounter();
    
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: Power ON!");
    /* XXX TODO NOTE. Until we get eHCILL setup, we can't turn off
 
     * the USART1. That would cause problems */
 
    /* stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_USART1); */
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_PWR);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
}

/*
 * Request a reset of the bluetooth module
 * Reset the chip by pulling nSHUTD high, sleep then low
 * Then we turn on the ext slow clock 32.768khz clock (MCO1)
 * then issue an HCI reset command.
 * The BT module should give a return of 7 bytes to ACK
 * RTS (Our CTS) will also get pulled LOW to ack reset
 */
uint8_t hw_bluetooth_power_cycle(void)
{
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: Reset...");
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_USART1);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_PWR);
    
    hw_bluetooth_clock_on();

    /* B12 nSHUTD LOW then HIGH (enable) */
    GPIO_ResetBits(GPIOB, BT_SHUTD);
    delay_ms(20);    
    GPIO_SetBits(GPIOB, BT_SHUTD);
    /* datasheet says at least 90ms to init */
    delay_ms(90);
    
    /* but lets sit and wait for the device to come up */
    for(int i = 0; i < 10000; i++)
    {
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0)
        {
            DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: Reset Success! Took %d ms", 90 + i);
            stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_USART1);
            stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
            stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
            stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_PWR);
            return 0;
        }
        delay_ms(1);
    }

/*     assert(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0 && "BT Reset Failed");*/
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_USART1);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_PWR);
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: Failed? TERMINAL!");
    return 1;
}


/*
 * IRQ trigger for EXT 11 for bluetooth
 * This is for low power shutdown wakeup
 * 
 * display has this too EXTI, but that is no longer used
 */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line11);
        DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "EXTI");
        bt_stack_cts_irq();
    }
    else if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        /* Display used me */
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}




/*
 * Configure and enable the IRQ for the Clear To Send interrupt
 * The stack will deep sleep bluetooth when idle, and any incoming packets will
 * call this interrupt so we can wake up and get ready
 */
void hw_bluetooth_enable_cts_irq()
{
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    
    NVIC_InitTypeDef nvic_init_struct;
    EXTI_InitTypeDef exti_init_struct;

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource11);
    
    exti_init_struct.EXTI_Line = EXTI_Line11;
    exti_init_struct.EXTI_LineCmd = ENABLE;
    exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init_struct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&exti_init_struct);

    nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 7;  // must be > 5
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: enabled CTS irq");
    
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
}

/*
 * When we dont need the deep sleep mode, we can ignore the CTS
 */
void hw_bluetooth_disable_cts_irq(void)
{
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    
    EXTI_InitTypeDef exti_init_struct;
    NVIC_InitTypeDef nvic_init_struct;
    
    exti_init_struct.EXTI_Line = EXTI_Line11;
    exti_init_struct.EXTI_LineCmd = DISABLE;
    exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init_struct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&exti_init_struct);

    /* CTS used PinSource11 which is connected to EXTI15_10 */
    nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 7;  // must be > 5
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init_struct.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvic_init_struct);
    DRV_LOG("BT", APP_LOG_LEVEL_DEBUG, "BT: disabled CTS irq");
    
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
}

stm32_usart_t *hw_bluetooth_get_usart(void)
{
    return &_usart1;
}

