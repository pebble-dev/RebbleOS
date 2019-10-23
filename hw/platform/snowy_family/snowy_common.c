/* snowy_common.c
 * Miscellaneous platform routines for Snowy family devices
  * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "log.h"
#include "stm32_power.h"
#include "stm32_buttons_platform.h"
#include "stm32_backlight.h"
#include "stm32_usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "snowy_vibrate.h"

// ENABLE this if you want smartstrap debugging output. For now if you do this qemu might not work
#define DEBUG_UART_SMARTSTRAP

void init_USART3(void);
void init_USART8(void);
void ss_debug_write(const unsigned char *p, size_t len);


/* Configs */
const vibrate_t hw_vibrate_config = {
    .pin     = GPIO_Pin_4,
    .port    = GPIOF,
    .clock   = RCC_AHB1Periph_GPIOF,
};

static const stm32_usart_config_t _usart3_config = {
    .usart                = USART3,
    .flow_control_enabled = FLOW_CONTROL_DISABLED,
    .usart_periph_bus     = STM32_POWER_APB1,
    .gpio_pin_tx_num      = 10,
    .gpio_pin_rx_num      = 11,
    .gpio_pin_rts_num     = 0,
    .gpio_pin_cts_num     = 0,
    .gpio_ptr             = GPIOC,
    .gpio_clock           = RCC_AHB1Periph_GPIOC,
    .usart_clock          = RCC_APB1Periph_USART3,
    .af                   = GPIO_AF_USART3,
};

static stm32_usart_t _usart3 = {
    &_usart3_config,
    NULL, /* no dma */
    230400
};

/* 
 * Begin device init 
 */
void debug_init()
{
    init_USART3(); // general debugging
#ifdef DEBUG_UART_SMARTSTRAP
    init_USART8(); // smartstrap debugging
#endif
    DRV_LOG("debug", APP_LOG_LEVEL_INFO, "Usart 3/8 Init");
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len)
{
    int i;
    
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_USART3);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
    
    for (i = 0; i < len; i++) {
        while (!(USART3->SR & USART_SR_TC));
        USART3->DR = p[i];
    }
    
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_USART3);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);

#ifdef DEBUG_UART_SMARTSTRAP
    ss_debug_write(p, len);
#endif
}

/* note that locking needs to be handled by external entity here */
void ss_debug_write(const unsigned char *p, size_t len)
{
#ifdef DEBUG_UART_SMARTSTRAP
    int i;
    UBaseType_t saved_int_status;

    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_UART8);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    
    for (i = 0; i < len; i++) {
        while (!(UART8->SR & USART_SR_TC));
        USART_SendData(UART8, p[i]);
    }
    
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_UART8);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
#endif
}

void log_clock_enable(void)
{
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_UART8);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
}

void log_clock_disable(void)
{
    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_UART8);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
void init_USART3(void)
{
    stm32_usart_init_device(&_usart3);
}

/*
 * Smartstrap port UART8 configured on GPIOE1
 */
void init_USART8(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_UART8);

    GPIO_InitStruct.GPIO_Pin = /*GPIO_Pin_0 |*/ GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_UART8);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_UART8);

    USART_InitStruct.USART_BaudRate = 230400;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx /* | USART_Mode_Rx */;
    USART_Init(UART8, &USART_InitStruct);
    USART_Cmd(UART8, ENABLE);

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_UART8);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE);
}

/* backlight */

#include "stm32_backlight_platform.h"

stm32_backlight_config_t platform_backlight = {
  .pin = GPIO_Pin_14,
  .tim = TIM12,
  .pin_source = GPIO_PinSource14,
  .port = RCC_AHB1Periph_GPIOB,
  .af = GPIO_AF_TIM12,
  .rcc_tim = RCC_APB1Periph_TIM12
};

/*
 * Initialise the system watchdog timer
 */
void hw_watchdog_init()
{
}

/*
 * Reset the watchdog timer
 */
void hw_watchdog_reset(void)
{
    IWDG_ReloadCounter();
}


static void  MemMang_Handler()
{
    KERN_LOG("MemHnd", APP_LOG_LEVEL_ERROR, "Memory manager Failed!");
    while(1);
}

void HardFault_Handler()
{
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "*** HARD FAULT ***");
    while(1);
}

void BusFault_Handler()
{
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "*** BUS FAULT ***");
    while(1);
}

void UsageFault_Handler_C(uint32_t *sp)
{
    uint16_t ufsr = *(uint16_t *)0xE000ED2A;
    
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "*** USAGE FAULT ***");
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx", sp[0], sp[1], sp[2], sp[3]);
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx", sp[4], sp[5], sp[6], (uint32_t) sp);
    SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "  UFSR: %04x", ufsr);
    
    if (ufsr & 1) {
        SYS_LOG("platform_snowy", APP_LOG_LEVEL_ERROR, "    *PC == %04x", *(uint16_t *)sp[6]);
    }
    while(1);
}

__attribute__((naked)) void UsageFault_Handler()
{
    asm volatile (
        "TST   LR, #4\n\t"
        "ITE   EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"
        "LDR   R1, =UsageFault_Handler_C\n\t"
        "BX    R1"
    );
}
