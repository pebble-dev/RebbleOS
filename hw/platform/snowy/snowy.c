/* snowy.c
 * Miscellaneous platform routines for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stdio.h"
#include "string.h"
#include "snowy.h"
#include "display.h"
#include "log.h"
#include "stm32_power.h"
#include "stm32_buttons_platform.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// ENABLE this if you want smartstrap debugging output. For now if you do this qemu might not work
#define DEBUG_UART_SMARTSTRAP

/*
 * Init HW
 */
void platform_init()
{
    SystemInit();

    stm32_power_init();
    
    // for stuff that needs special init for the platform
    // not done in SystemInit, it did something weird.
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);

    // set the default pri groups for the interrupts
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
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
    printf("c     : %ld", SystemCoreClock);
    printf("SYSCLK: %ld\n", RCC_Clocks.SYSCLK_Frequency);
    printf("CFGR  : %ld\n", RCC->PLLCFGR);
    printf("Relocating NVIC to 0x08004000\n");
}

/* Snowy platform button definitions */
const stm32_button_t platform_buttons[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK]   = { GPIO_Pin_4, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource4, RCC_AHB1Periph_GPIOG, EXTI4_IRQn },
    [HW_BUTTON_UP]     = { GPIO_Pin_3, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource3, RCC_AHB1Periph_GPIOG, EXTI3_IRQn },
    [HW_BUTTON_SELECT] = { GPIO_Pin_1, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource1, RCC_AHB1Periph_GPIOG, EXTI1_IRQn },
    [HW_BUTTON_DOWN]   = { GPIO_Pin_2, GPIOG, EXTI_PortSourceGPIOG, EXTI_PinSource2, RCC_AHB1Periph_GPIOG, EXTI2_IRQn }
};

STM32_BUTTONS_MK_IRQ_HANDLER(1)
STM32_BUTTONS_MK_IRQ_HANDLER(2)
STM32_BUTTONS_MK_IRQ_HANDLER(3)
STM32_BUTTONS_MK_IRQ_HANDLER(4)

