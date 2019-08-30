/* asterix.c
 * boot routines for aWatch2
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "nrf_delay.h"
#include "nrfx_uart.h"
#include "nrfx_twi.h"
#include "nrf_gpio.h"

#include "board_config.h"

/*** PMIC ***/

/* PMIC code lives here for the time being. */

static nrfx_twi_t pmic_twi = NRFX_TWI_INSTANCE(0);

static void i2c_write(const nrfx_twi_t *bus, uint8_t dev, uint8_t addr, uint8_t datum) {
    nrfx_err_t err;
    uint8_t data[2];
    
    data[0] = addr;
    data[1] = datum;
    
    err = nrfx_twi_tx(bus, dev, data, 2, false);
    assert(err == NRFX_SUCCESS);
}

static void pmic_init() {
#ifdef ASTERIX_BOARD_ASTERIX
    nrfx_err_t err;

    const nrfx_twi_config_t config = {
        .scl = 4,
        .sda = 5,
        .frequency = NRF_TWI_FREQ_400K,
        .interrupt_priority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1,
        .hold_bus_uninit = true
    };
    
    err = nrfx_twi_init(&pmic_twi, &config, NULL, NULL);
    assert(err == NRFX_SUCCESS);

    nrfx_twi_enable(&pmic_twi);
    
    /* XXX: these should live in a real PMIC driver */
    i2c_write(&pmic_twi, 0x28, 0x1E, 0x81); /* Don't panic, little Maxim. */
    i2c_write(&pmic_twi, 0x28, 0x13, 0x19); /* LDO1 3.3v */
    i2c_write(&pmic_twi, 0x28, 0x12, 0x02); /* LDO1 enable */
    i2c_write(&pmic_twi, 0x28, 0x0F, 0xE8); /* Buck2 enable, burst mode, 2.2uH */
    i2c_write(&pmic_twi, 0x28, 0x10, 0x28); /* buck2, 2.8v */
#endif
}

/*** platform ***/

void platform_init() {
    pmic_init();
    nrf_gpio_cfg_output(BOARD_BACKLIGHT_PIN);
    nrf_gpio_pin_set(BOARD_BACKLIGHT_PIN);
#ifdef ASTERIX_BOARD_VLA_DVB1
    /* shared SPI chip selects */
    nrf_gpio_cfg_output(15);
    nrf_gpio_cfg_output(16);
    nrf_gpio_pin_set(15);
    nrf_gpio_pin_set(16);
#endif
}

void platform_init_late() {
    nrf_gpio_pin_clear(BOARD_BACKLIGHT_PIN);
}

/*** watchdog timer ***/

void hw_watchdog_init() {
}

void hw_watchdog_reset() {
}

/*** ambient light sensor ***/

void hw_ambient_init() {
}

uint16_t hw_ambient_get() {
    return 0;
}


/* buttons */

/* backlight */

/* vibrate */

void hw_vibrate_init() {
}

void hw_vibrate_enable(uint8_t enabled) {
}

void hw_power_init()
{
}

uint16_t hw_power_get_bat_mv(void)
{
    return 0;
}

uint8_t hw_power_get_chg_status(void)
{
    return 0;
}


void hw_backlight_init()
{
}

void hw_backlight_set() { }

void rtc_init() { }

static struct tm tm;
struct tm *hw_get_time(void) { return &tm; }

void hw_set_time(struct tm *tm) { }

