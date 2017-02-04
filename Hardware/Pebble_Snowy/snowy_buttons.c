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
#include "stdio.h"
#include "snowy_buttons.h"
#include "buttons.h"

buttons_t buttons = {
    .Back   = { GPIO_Pin_4, GPIOG },
    .Up     = { GPIO_Pin_3, GPIOG },
    .Select = { GPIO_Pin_1, GPIOG },
    .Down   = { GPIO_Pin_2, GPIOG }
};

/*
 * Initialise all buttons.
 * They are connected by each their own interrupt.
 */
void hw_buttons_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure2;
    
    // For snowy we can shortcut the port init. For other boards each bank port will need a declaration for each unique bank
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure2.GPIO_Pin =  buttons.Back.Pin | buttons.Select.Pin | buttons.Up.Pin | buttons.Down.Pin;
    GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(buttons.Back.Port, &GPIO_InitStructure2);
        
    // setup the pin interrupts
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line4, EXTI_PinSource4, EXTI4_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line3, EXTI_PinSource3, EXTI3_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line2, EXTI_PinSource2, EXTI2_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line1, EXTI_PinSource1, EXTI1_IRQn);
}

/*
 * Connect each supplied port and line to the interrupt NVIC
 */
void buttons_init_intn(uint32_t EXTIport, uint32_t EXTIline, uint32_t pinSource, uint32_t EXTI_irq)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    SYSCFG_EXTILineConfig(EXTIport, pinSource);
    
    EXTI_InitStruct.EXTI_Line = EXTIline;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&EXTI_InitStruct);
 
    // Add IRQ vector to NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI_irq;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/* 
 * Set interrupt handlers. Each just calls the isr.
 */

void buttons_handle_ISR(uint32_t line, button_t *button)
{
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(line) != RESET)
    {       
        button_isr(button);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(line);
    }
}

void EXTI4_IRQHandler(void)
{
    //buttons_handle_ISR(EXTI_Line4, &buttons.Back);
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {       
        button_isr(&buttons.Back);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI3_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {       
        button_isr(&buttons.Up);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI2_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {       
        button_isr(&buttons.Down);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI1_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {       
        button_isr(&buttons.Select);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

uint8_t hw_button_pressed(button_t *button)
{
    int btn = GPIO_ReadInputDataBit(button->Port, button->Pin);
    
    return btn;
}

