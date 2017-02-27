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

/*
 * Printf support. Uncommet to disable output
 * (but not remove strings)
 */
//#define printf(fmt, ...) (0)

/* 
 * Begin device init 
 */
void debug_init()
{
    init_USART3(); // general debugging
    init_USART8(); // smartstrap debugging
        
    printf("USART3/8 init\n");
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
    // for stuff that needs special init for the platform
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
