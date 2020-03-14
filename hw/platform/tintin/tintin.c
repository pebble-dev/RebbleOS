/* tintin.c
 * miscellaneous routines for tintin-like devices
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stm32f2xx.h>
#include "tintin.h"
#include <debug.h>

#include <stm32f2xx_usart.h>
#include <stm32f2xx_gpio.h>
#include <stm32f2xx_spi.h>
#include <stm32f2xx_iwdg.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>
#include "rebbleos.h"

#include "stm32_usart.h"
#include "stm32_power.h"
#include "stm32_rtc.h"
#include "stm32_backlight.h"
#include "stm32_spi.h"
#include "stm32_cc256x.h"
#include "btstack_rebble.h"

// extern void *strcpy(char *a2, const char *a1);

/*** debug routines ***/

static inline void _init_USART3();
static int _debug_initialized;


static const stm32_usart_config_t _u3_config = {
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
    &_u3_config,
    NULL,
    230400
};


void debug_init() {
    _init_USART3();
    _debug_initialized = 1;
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len) {
    int i;

    if (!_debug_initialized)
        return;

    stm32_usart_write(&_usart3, p, len);
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
static inline void _init_USART3(void)
{
    /* leave the usart clock on */
    stm32_power_request(_usart3.config->usart_periph_bus, _usart3.config->usart_clock);
    stm32_usart_init_device(&_usart3);    
}

/*** platform ***/

void platform_init() {
    SystemInit();

    stm32_power_init();
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void platform_init_late() {
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_INFO, "tintin: late init");
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_INFO, "c     : %lu", SystemCoreClock);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_INFO, "SYSCLK: %lu\n", RCC_Clocks.SYSCLK_Frequency);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_INFO, "CFGR  : %lx\n", RCC->PLLCFGR);
}

/*** watchdog timer ***/

void hw_watchdog_init() {
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(0xFFF);
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
    IWDG_Enable();
    IWDG_ReloadCounter();
}

void hw_watchdog_reset() {
    /* We're exceedingly careful in this.  We turn on GPIO clocks, read the
     * GPIO to see if the back button is not pressed, read the GPIO clock
     * register to see if we got into trouble, and if and only if we meet
     * all of those, we feed the watchdog.  */
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
    
    if (!(GPIOC->IDR & (1 << 3))) /* pressed? */
        goto nofeed;
    if ((GPIOC->MODER & (0x3 << (3 * 2))) != (0x0 << (3 * 2)))
        goto nofeed;
    if ((GPIOC->PUPDR & (0x3 << (3 * 2))) != (0x1 << (3 * 2)))
        goto nofeed;
    
    if (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN)) /* GPIOC clocks? */
        goto nofeed;
    if (RCC->AHB1RSTR & RCC_AHB1RSTR_GPIOCRST) /* in reset? */
        goto nofeed;

    IWDG_ReloadCounter();
    
nofeed:
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
}

/*** ambient light sensor ***/

void hw_ambient_init() {
}

uint16_t hw_ambient_get() {
    return 0;
}


/* buttons */

#include "stm32_buttons_platform.h"

const stm32_button_t platform_buttons[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK]   = { GPIO_Pin_3, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource3, RCC_AHB1Periph_GPIOC, EXTI3_IRQn },
    [HW_BUTTON_UP]     = { GPIO_Pin_2, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource2, RCC_AHB1Periph_GPIOA, EXTI2_IRQn },
    [HW_BUTTON_SELECT] = { GPIO_Pin_6, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource6, RCC_AHB1Periph_GPIOC, EXTI9_5_IRQn },
    [HW_BUTTON_DOWN]   = { GPIO_Pin_1, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource1, RCC_AHB1Periph_GPIOA, EXTI1_IRQn }
};

STM32_BUTTONS_MK_IRQ_HANDLER(3)
STM32_BUTTONS_MK_IRQ_HANDLER(2)
STM32_BUTTONS_MK_IRQ_HANDLER(9_5)
STM32_BUTTONS_MK_IRQ_HANDLER(1)

/* backlight */

#include "stm32_backlight_platform.h"

stm32_backlight_config_t platform_backlight = {
  .pin = GPIO_Pin_5,
  .tim = TIM3,
  .pin_source = GPIO_PinSource5,
  .port = RCC_AHB1Periph_GPIOB,
  .af = GPIO_AF_TIM3,
  .rcc_tim = RCC_APB1Periph_TIM3
};


/* vibrate */

void hw_vibrate_init() {
}

void hw_vibrate_enable(uint8_t enabled) {
}



void ss_debug_write(const unsigned char *p, size_t len)
{
    // unsupported on this platform
}

void log_clock_enable() {
}
void log_clock_disable() {
}


void hw_power_init()
{
}

uint16_t hw_power_get_bat_mv(void)
{
    
}

uint8_t hw_power_get_chg_status(void)
{
}

void HardFault_Handler(uint32_t *sp)
{
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "*** HARD FAULT ***");
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx", sp[0], sp[1], sp[2], sp[3]);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx", sp[4], sp[5], sp[6], (uint32_t) sp);
    while(1);
}

void BusFault_Handler()
{
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "*** BUS FAULT ***");
    while(1);
}

void UsageFault_Handler_C(uint32_t *sp)
{
    uint16_t ufsr = *(uint16_t *)0xE000ED2A;
    
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "*** USAGE FAULT ***");
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx", sp[0], sp[1], sp[2], sp[3]);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx", sp[4], sp[5], sp[6], (uint32_t) sp);
    SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "  UFSR: %04x", ufsr);
    
    if (ufsr & 1) {
        SYS_LOG("platform_tintin", APP_LOG_LEVEL_ERROR, "    *PC == %04x", *(uint16_t *)sp[6]);
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
