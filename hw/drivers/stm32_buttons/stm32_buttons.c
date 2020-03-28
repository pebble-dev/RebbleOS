/* 
 * stm32_buttons.c
 * Implementation of generic STM32 button HAL.
 * RebbleOS
 *
 * Barry Carter <barry.carter@gmail.com>
 * Joshua Wise <joshua@joshuawise.com>
 *
 * The stm32_buttons implementation comes in two major parts: the HAL
 * implementation (in this directory, which provides the hw_buttons_* APIs
 * to the OS implementation), and the platform-specific constants (i.e.,
 * which GPIOs are associated with which button, etcetera).  As such, there
 * are two header files involved: stm32_buttons.h, which provides the OS HAL
 * (and is expected to be included by a platform's platform.h), and
 * stm32_buttons_platform.h, which is intended to be included by a file in
 * hw/platform/.../, and which provides typedefs for button constants.
 */

#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_gpio.h"
#    include "stm32f2xx_exti.h"
#    include "stm32f2xx_syscfg.h"
#    include "stm32f2xx_rcc.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include "stdio.h"
#include "stm32_buttons.h"
#include "stm32_power.h"
#include "stm32_buttons_platform.h"
#include "debug.h"

static hw_button_isr_t _isr = NULL;

/* On stm32, we initialize each button in sequence. */
void hw_button_init(void)
{
    for (int i = 0; i < HW_BUTTON_MAX; i++) {
        const stm32_button_t *btn = &platform_buttons[i];
        
        /* Initialize the GPIO. */
        GPIO_InitTypeDef gpioinit;
        stm32_power_request(STM32_POWER_AHB1, btn->gpio_clock);
        
        gpioinit.GPIO_Mode  = GPIO_Mode_IN;
        gpioinit.GPIO_Pin   = btn->gpio_pin;
        gpioinit.GPIO_PuPd  = GPIO_PuPd_UP;
        gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
        gpioinit.GPIO_OType = GPIO_OType_OD;
        GPIO_Init(btn->gpio_ptr, &gpioinit);

        stm32_power_release(STM32_POWER_AHB1, btn->gpio_clock);

        /* Set up the pin external interrupt. */
        EXTI_InitTypeDef extiinit;
        
        stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
        
        SYSCFG_EXTILineConfig(btn->exti_port, btn->exti_pinsource);
        
        extiinit.EXTI_Line = 1 << btn->exti_pinsource;
        extiinit.EXTI_LineCmd = ENABLE;
        extiinit.EXTI_Mode = EXTI_Mode_Interrupt;
        extiinit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        EXTI_Init(&extiinit);
        
        /* And turn on the interrupt in the interrupt controller. */
        NVIC_InitTypeDef nvicinit;
        
        nvicinit.NVIC_IRQChannel = btn->exti_irq;
        nvicinit.NVIC_IRQChannelPreemptionPriority = 5;
        nvicinit.NVIC_IRQChannelSubPriority = 0x00;
        nvicinit.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvicinit);

        stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
    }
}

void hw_button_set_isr(hw_button_isr_t isr)
{
    _isr = isr;
}

/* The master button handler ISR polls each button's interrupt, rather than
 * making the macro figure out which stm32_button_t we really mean..
 */
void stm32_buttons_raw_isr()
{
    for (int i = 0; i < HW_BUTTON_MAX; i++) {
        const stm32_button_t *btn = &platform_buttons[i];
        uint32_t exti_line = 1 << btn->exti_pinsource;
        
        if (EXTI_GetITStatus(exti_line) == RESET)
            continue;
        
        /* Handle the interrupt for button i. */
        if (_isr)
            _isr(i);
        
        EXTI_ClearITPendingBit(exti_line);
    }
}

int hw_button_pressed(hw_button_t button_id)
{
    assert(button_id < HW_BUTTON_MAX);
    const stm32_button_t *btn = &platform_buttons[button_id];
    
    /* XXX: should push and pop.  calling this from an ISR could be real exciting! */
    stm32_power_request(STM32_POWER_AHB1, btn->gpio_clock);
    int stat = GPIO_ReadInputDataBit(btn->gpio_ptr, btn->gpio_pin);
    stm32_power_release(STM32_POWER_AHB1, btn->gpio_clock);
    
    return !stat;
}
