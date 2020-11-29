/* debug.c
 * Debug UART routines for nRF52840-based platforms
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "nrf_delay.h"
#include "nrfx_uart.h"
#include "board_config.h"

void delay_us(int us) {
    nrf_delay_us(us);
}

/* We use UART, instead of UARTE, because we could be writing from flash,
 * which would cause the EasyDMA engine to lock up, and that would be bad. 
 * So we just take the performance hit and run a UART engine in PIO mode. 
 * So it goes.  */

static nrfx_uart_t debug_uart = NRFX_UART_INSTANCE(0);

__attribute__((naked)) uint32_t get_msp()
{
    asm volatile(
        "mrs r0, msp\n"
        "bx lr\n"
    );
}

static int _did_init = 0;

void debug_init() {
    nrfx_err_t err;
    
    nrfx_uart_config_t config = NRFX_UART_DEFAULT_CONFIG;
    config.pseltxd = BOARD_DEBUG_UART_TXD;
    
    if (_did_init)
        nrfx_uart_uninit(&debug_uart);
    _did_init = 1;
    
    err = nrfx_uart_init(&debug_uart, &config, NULL /* nonblocking event handler */);
    assert(err == NRFX_SUCCESS);
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len) {
    nrfx_err_t err;
    
    while (len) {
        /* Sigh... */
        if (*p == '\n') {
            unsigned char c = '\r';
            err = nrfx_uart_tx(&debug_uart, &c, 1);
            /* assert(err == NRFX_SUCCESS); */
        }
        
        err = nrfx_uart_tx(&debug_uart, p, 1);
        /* assert(err == NRFX_SUCCESS); */
        
        len--;
        p++;
    }
}

void ss_debug_write(const unsigned char *p, size_t len)
{
    // unsupported on this platform
}

void log_clock_enable() {
}

void log_clock_disable() {
}
