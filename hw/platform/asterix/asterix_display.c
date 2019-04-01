/* asterix_display.c
 * Display driver for LS013B7DH05 attached via SPI to nRF52840
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"

#define BOARD_DISPLAY_PIN_MOSI 15
#define BOARD_DISPLAY_PIN_SCK  14
#define BOARD_DISPLAY_PIN_SCS  16

static uint8_t _display_fb[168][20];
static nrfx_spim_t display_spi = NRFX_SPIM_INSTANCE(3);

void hw_display_init() {
    nrfx_err_t err;
    
    nrfx_spim_config_t config = NRFX_SPIM_DEFAULT_CONFIG;
    config.mosi_pin = BOARD_DISPLAY_PIN_MOSI;
    config.sck_pin = BOARD_DISPLAY_PIN_SCK;
    config.ss_pin = BOARD_DISPLAY_PIN_SCS;
    config.ss_active_high = true;
    config.use_hw_ss = true; /* only available with SPIM3 */
    config.frequency = NRF_SPIM_FREQ_1M;
    config.irq_priority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    
    err = nrfx_spim_init(&display_spi, &config, NULL, NULL);
    assert(err == NRFX_SUCCESS);
    
    DRV_LOG("asterix", APP_LOG_LEVEL_INFO, "asterix: hw_display_init");
}

void hw_display_reset() {
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
    display_done_isr(0);
}

uint8_t *hw_display_get_buffer(void) {
    return (uint8_t *)_display_fb;
}

uint8_t hw_display_process_isr() {
    /* We do the entire thing in one go, polling, for right now. */
    nrfx_err_t err;
    
    for (int i = 0; i < 168; i++) {
        uint8_t buf[22];
        nrfx_spim_xfer_desc_t desc = NRFX_SPIM_XFER_TX(buf, 22);
        
        buf[0] = 0x80;
        buf[1] = __RBIT(__REV(168 - i));
        for (int j = 0; j < 18; j++)
            buf[2+j] = _display_fb[i][17-j];
        buf[21] = 0;
        
        err = nrfx_spim_xfer(&display_spi, &desc, 0);
        assert(err == NRFX_SUCCESS);
    }
    return 1;
}
