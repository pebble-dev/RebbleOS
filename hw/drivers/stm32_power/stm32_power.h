/* 
 * stm32_buttons.h
 * HAL-facing API for stm32 power management
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * See stm32_power.c for a better description of this file.
 */

#ifndef __STM32_POWER_H
#define __STM32_POWER_H

/* STM32_POWER_EXPANDO is an expansion macro for all of the clock registers
 * on the system.  It takes as a parameter one macro, X, which takes two
 * parameters of its own:
 *   name -- name of the clock register
 *   bits -- number of bits to the highest bit that can be set in the clock
 *           register (i.e., on stm32f2xx, AHB2 would have 8 bits, because
 *           the highest bit set is RCC_AHB2Periph_OTG_FS, which is 0x80)
 */
 
#if defined(STM32F4XX)
#    define STM32_POWER_EXPANDO(X) \
       X(AHB1, 32) \
       X(AHB2, 8) \
       X(AHB3, 2) \
       X(APB1, 32) \
       X(APB2, 28)
#elif defined(STM32F2XX)
#    define STM32_POWER_EXPANDO(X) \
       X(AHB1, 31) \
       X(AHB2, 8) \
       X(AHB3, 1) \
       X(APB1, 30) \
       X(APB2, 18)
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif

typedef enum stm32_power_register {
#define MK_ENUM(n, b) STM32_POWER_##n,
    STM32_POWER_EXPANDO(MK_ENUM)
#undef MK_ENUM
    STM32_POWER_MAX
} stm32_power_register_t;

void stm32_power_incr(stm32_power_register_t reg, uint32_t domain, int incr);

static inline void stm32_power_request(stm32_power_register_t reg, uint32_t domain) {
    stm32_power_incr(reg, domain, 1);
}

static inline void stm32_power_release(stm32_power_register_t reg, uint32_t domain) {
    stm32_power_incr(reg, domain, -1);
}

#endif
