/* nrf52_ls013b7dh05.c
 * Display driver for LS013B7DH05 attached via SPI to nRF52840
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <assert.h>
#include "FreeRTOSConfig.h"
#include "log.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "board_config.h"

#define DISPLAY_LINES_PER_CHUNK 21 /* 8 chunks per frame */

static void _spi_handler(const nrfx_spim_evt_t *p_event, void *p_context) {
    display_done_isr(0);
}

/* XXX: We have enough padding here to do this all in-place... */
static uint8_t _display_fb[168][20];
static nrfx_spim_t _display_spi = NRFX_SPIM_INSTANCE(3);
static int _display_curline = 0;

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
    
    err = nrfx_spim_init(&_display_spi, &config, _spi_handler, NULL);
    assert(err == NRFX_SUCCESS);
    
    DRV_LOG("hw", APP_LOG_LEVEL_INFO, "nrf52_ls013b7dh05: hw_display_init");
}

void hw_display_reset() {
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
    _display_curline = 0;
    display_done_isr(0);
}

uint8_t *hw_display_get_buffer(void) {
    return (uint8_t *)_display_fb;
}

static uint8_t _dispbuf[DISPLAY_LINES_PER_CHUNK * 20 + 2];
static nrfx_spim_xfer_desc_t _spi_desc = NRFX_SPIM_XFER_TX(_dispbuf, 0);

uint8_t hw_display_process_isr() {
    nrfx_err_t err;
    
    if (_display_curline == 168)
        return 1; 
    
    int p = 0;
    _dispbuf[p++] = 0x80;
    for (int i = 0; i < DISPLAY_LINES_PER_CHUNK; i++) {
#ifdef BOARD_DISPLAY_ROT180
        _dispbuf[p++] = __RBIT(__REV(_display_curline + 1));
        for (int j = 0; j < 18; j++)
            _dispbuf[p++] = __RBIT(__REV(_display_fb[_display_curline][j]));
#else
        _dispbuf[p++] = __RBIT(__REV(168 - _display_curline));
        for (int j = 0; j < 18; j++)
            _dispbuf[p++] = _display_fb[_display_curline][17-j];
#endif
        _dispbuf[p++] = 0;
        _display_curline++;
    }
    _dispbuf[p++] = 0;

    _spi_desc.tx_length = p;
    err = nrfx_spim_xfer(&_display_spi, &_spi_desc, 0);
    assert(err == NRFX_SUCCESS);
    
    return 0;
}
