/* 
 * nrf52_buttons.c
 * Implementation of generic nRF52 button HAL.
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * The nrf52_buttons implementation is implemented in only one file, which
 * provides the OS HAL interface (and whose header is expected to be
 * included by a platform's platform.h).  Configuration for the button
 * driver is done by board_config.h, which is expected to define
 * BOARD_BUTTON_{BACK,UP,SELECT,DOWN}_PIN constants (which are
 */

#include <debug.h>
#include "nrf52_buttons.h"
#include "nrfx_gpiote.h"
#include "board_config.h"

#define BOARD_BUTTON_BACK_PIN   NRF_GPIO_PIN_MAP(1,01)
#define BOARD_BUTTON_UP_PIN     NRF_GPIO_PIN_MAP(1,02)
#define BOARD_BUTTON_SELECT_PIN NRF_GPIO_PIN_MAP(1,03)
#define BOARD_BUTTON_DOWN_PIN   NRF_GPIO_PIN_MAP(1,04)

static nrfx_gpiote_pin_t nrf52_buttons_pindefs[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK] = BOARD_BUTTON_BACK_PIN,
    [HW_BUTTON_UP] = BOARD_BUTTON_UP_PIN,
    [HW_BUTTON_SELECT] = BOARD_BUTTON_SELECT_PIN,
    [HW_BUTTON_DOWN] = BOARD_BUTTON_DOWN_PIN,
};

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
