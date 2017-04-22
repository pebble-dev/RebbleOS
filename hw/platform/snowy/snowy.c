/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "snowy_rtc.h"
#include "stdio.h"
#include "string.h"
#include "snowy.h"
#include "stm32_power.h"
#include "stm32_buttons_platform.h"

/*
 * Printf support. Uncommet to disable output
 * (but not remove strings)
 */
//#define printf(fmt, ...) (0)

// ENABLE this if you want smartstrap debugging output. For now if you do this qemu might not work
#define DEBUG_UART_SMARTSTRAP

/* 
 * Begin device init 
 */
void debug_init()
{
    init_USART3(); // general debugging
#ifdef DEBUG_UART_SMARTSTRAP
    init_USART8(); // smartstrap debugging
#endif
    printf("USART3/8 init\n");
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len)
{
    int i;
    /* XXX: better power management */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    for (i = 0; i < len; i++) {
        while (!(USART3->SR & USART_SR_TC));
        USART3->DR = p[i];
    }
    
// #ifdef DEBUG_UART_SMARTSTRAP
//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
//     
//     for (i = 0; i < len; i++) {
//         while (!(UART8->SR & USART_SR_TC));
//         UART8->DR = p[i];
//     }
// #endif
}

/* note that locking needs to be handled by external entity here */
void ss_debug_write(const unsigned char *p, size_t len)
{
    int i;
#ifdef DEBUG_UART_SMARTSTRAP
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    
    for (i = 0; i < len; i++) {
        while (!(UART8->SR & USART_SR_TC));
        UART8->DR = p[i];
    }
#endif
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
void init_USART3(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStruct);
    USART_Cmd(USART3, ENABLE);
}

/*
 * Smartstrap port UART8 configured on GPIOE1
 */
void init_USART8(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_UART8);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_UART8);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART8, &USART_InitStruct);
    USART_Cmd(UART8, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
}

/*
 * Init HW
 */
void platform_init()
{
    stm32_power_init();
    
    // for stuff that needs special init for the platform
    // not done in SystemInit, it did something weird.
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);

    // set the default pri groups for the interrupts
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void platform_init_late()
{
    // it needs a gentle reminder
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // ginge: the below may only apply to non-snowy
    // joshua - Yesterday at 8:42 PM
    // I recommend setting RTCbackup[0] 0x20000 every time you boot
    // that'll force kick you into PRF on teh next time you enter the bootloader

    // Dump clocks
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    printf("c     : %d", SystemCoreClock);
    printf("SYSCLK: %d\n", RCC_Clocks.SYSCLK_Frequency);
    printf("CFGR  : %d\n", RCC->PLLCFGR);
    printf("Relocating NVIC to 0x08004000\n");
}

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

/* Snowy platform button definitions */
stm32_button_t platform_buttons[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK]   = { GPIO_Pin_4, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource4, RCC_AHB1Periph_GPIOG, EXTI4_IRQn },
    [HW_BUTTON_UP]     = { GPIO_Pin_3, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource3, RCC_AHB1Periph_GPIOG, EXTI3_IRQn },
    [HW_BUTTON_SELECT] = { GPIO_Pin_1, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource1, RCC_AHB1Periph_GPIOG, EXTI1_IRQn },
    [HW_BUTTON_DOWN]   = { GPIO_Pin_2, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource2, RCC_AHB1Periph_GPIOG, EXTI2_IRQn }
};

STM32_BUTTONS_MK_IRQ_HANDLER(1)
STM32_BUTTONS_MK_IRQ_HANDLER(2)
STM32_BUTTONS_MK_IRQ_HANDLER(3)
STM32_BUTTONS_MK_IRQ_HANDLER(4)



static void  MemMang_Handler()
{
    printf("hard\n");
    while(1);
}
