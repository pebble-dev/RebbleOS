/* asterix_buttons.c
 * buttons for Asterix
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "nrf52_buttons_platform.h"

#define BOARD_BUTTON_BACK_PIN   NRF_GPIO_PIN_MAP(1,01)
#define BOARD_BUTTON_UP_PIN     NRF_GPIO_PIN_MAP(1,02)
#define BOARD_BUTTON_SELECT_PIN NRF_GPIO_PIN_MAP(1,03)
#define BOARD_BUTTON_DOWN_PIN   NRF_GPIO_PIN_MAP(1,04)

nrfx_gpiote_pin_t nrf52_buttons_pindefs[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK] = BOARD_BUTTON_BACK_PIN,
    [HW_BUTTON_UP] = BOARD_BUTTON_UP_PIN,
    [HW_BUTTON_SELECT] = BOARD_BUTTON_SELECT_PIN,
    [HW_BUTTON_DOWN] = BOARD_BUTTON_DOWN_PIN,
};
