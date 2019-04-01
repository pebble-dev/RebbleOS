/* asterix_buttons.c
 * buttons for nRF52840
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "nrfx_gpiote.h"

#define BOARD_BUTTON_BACK_PIN   NRF_GPIO_PIN_MAP(1,01)
#define BOARD_BUTTON_UP_PIN     NRF_GPIO_PIN_MAP(1,02)
#define BOARD_BUTTON_SELECT_PIN NRF_GPIO_PIN_MAP(1,03)
#define BOARD_BUTTON_DOWN_PIN   NRF_GPIO_PIN_MAP(1,04)

struct pins {
    nrfx_gpiote_pin_t pin;
};

static nrfx_gpiote_pin_t pindefs[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK] = BOARD_BUTTON_BACK_PIN,
    [HW_BUTTON_UP] = BOARD_BUTTON_UP_PIN,
    [HW_BUTTON_SELECT] = BOARD_BUTTON_SELECT_PIN,
    [HW_BUTTON_DOWN] = BOARD_BUTTON_DOWN_PIN,
};

static hw_button_isr_t _isr;

static void _button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    for (int i = 0; i < HW_BUTTON_MAX; i++)
        if (pindefs[i] == pin)
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
        err = nrfx_gpiote_in_init(pindefs[i], &config, _button_handler);
        assert(err == NRFX_SUCCESS);
        
        nrfx_gpiote_in_event_enable(pindefs[i], 1);
    }
}

void hw_button_set_isr(hw_button_isr_t isr) {
    _isr = isr;
}

int hw_button_pressed(hw_button_t button_id) {
    return !nrfx_gpiote_in_is_set(pindefs[button_id]);
}
