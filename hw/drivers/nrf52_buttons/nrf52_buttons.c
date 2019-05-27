/* 
 * nrf52_buttons.c
 * Implementation of generic nRF52 button HAL.
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * The nrf52_buttons implementation comes in two major parts: the HAL
 * implementation (in this directory, which provides the hw_buttons_* APIs
 * to the OS implementation), and the platform-specific constants (i.e.,
 * which GPIOs are associated with which button, etcetera).  As such, there
 * are two header files involved: nrf52_buttons.h, which provides the OS HAL
 * (and is expected to be included by a platform's platform.h), and
 * nrf52_buttons_platform.h, which is intended to be included by a file in
 * hw/platform/.../, and which provides typedefs for button constants.
 */

#include <debug.h>
#include "nrf52_buttons_platform.h"

static hw_button_isr_t _isr;

static void _button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    for (int i = 0; i < HW_BUTTON_MAX; i++)
        if (nrf52_buttons_pindefs[i] == pin)
            _isr(i);
}

void hw_button_init(void) {
    nrfx_err_t err;
    
    err = nrfx_gpiote_init();
    assert(err == NRFX_SUCCESS);
    
    nrfx_gpiote_in_config_t config;
    config.is_watcher = false;
    config.hi_accuracy = false;
    config.pull = NRF_GPIO_PIN_PULLUP;
    config.sense = NRF_GPIOTE_POLARITY_TOGGLE;
    config.skip_gpio_setup = false;
    
    for (int i = 0; i < HW_BUTTON_MAX; i++) {
        err = nrfx_gpiote_in_init(nrf52_buttons_pindefs[i], &config, _button_handler);
        assert(err == NRFX_SUCCESS);
        
        nrfx_gpiote_in_event_enable(nrf52_buttons_pindefs[i], 1);
    }
}

void hw_button_set_isr(hw_button_isr_t isr) {
    _isr = isr;
}

int hw_button_pressed(hw_button_t button_id) {
    return !nrfx_gpiote_in_is_set(nrf52_buttons_pindefs[button_id]);
}
