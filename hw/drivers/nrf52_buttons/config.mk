CFLAGS_driver_nrf52_buttons = -Ihw/drivers/nrf52_buttons

SRCS_driver_nrf52_buttons = hw/drivers/nrf52_buttons/nrf52_buttons.c
SRCS_driver_nrf52_buttons += modules/nrfx/drivers/src/nrfx_gpiote.c
