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

// extern void *strcpy(char *a2, const char *a1);

/*** debug routines ***/

/* We use UART, instead of UARTE, because we could be writing from flash,
 * which would cause the EasyDMA engine to lock up, and that would be bad. 
 * So we just take the performance hit and run a UART engine in PIO mode. 
 * So it goes.  */

static nrfx_uart_t debug_uart = NRFX_UART_INSTANCE(0);

void debug_init() {
    nrfx_err_t err;
    
    nrfx_uart_config_t config = NRFX_UART_DEFAULT_CONFIG;
    config.pseltxd = 8;
    
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
            assert(err == NRFX_SUCCESS);
        }
        
        err = nrfx_uart_tx(&debug_uart, p, 1);
        assert(err == NRFX_SUCCESS);
        
        len--;
        p++;
    }
}

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
}

/*** platform ***/

void platform_init() {
    pmic_init();
    nrf_gpio_cfg_output(13);
    nrf_gpio_pin_set(13);
}

void platform_init_late() {
    nrf_gpio_pin_clear(13);
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



void ss_debug_write(const unsigned char *p, size_t len)
{
    // unsupported on this platform
}

void log_clock_enable() {
}
void log_clock_disable() {
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

void HardFault_Handler(uint32_t *sp)
{
    printf("*** HARD FAULT ***\n");
    printf("   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx\n", sp[4], sp[5], sp[6], (uint32_t) sp);
    while(1);
}

void BusFault_Handler()
{
    printf("*** BUS FAULT ***\n");
    while(1);
}

void UsageFault_Handler_C(uint32_t *sp)
{
    uint16_t ufsr = *(uint16_t *)0xE000ED2A;
    
    printf("*** USAGE FAULT ***\n");
    printf("   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx\n", sp[4], sp[5], sp[6], (uint32_t) sp);
    printf("  UFSR: %04x\n", ufsr);
    
    if (ufsr & 1) {
        printf("    *PC == %04x\n", *(uint16_t *)sp[6]);
    }
    while(1);
}

__attribute__((naked)) void UsageFault_Handler()
{
    asm volatile (
        "TST   LR, #4\n\t"
        "ITE   EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"
        "LDR   R1, =UsageFault_Handler_C\n\t"
        "BX    R1"
    );
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    printf("*** error %ld (pc %lx, info %lx)\n", id, pc, info);
    panic("app_error_fault_handler from nRF SDK");
}

void app_error_handler_bare(ret_code_t error_code) 
{
    app_error_fault_handler(error_code, 0, 0);
}

void hw_backlight_init()
{
    bluetooth_init_complete(-1);
}

void hw_backlight_set() { }

void delay_us(int us) {
    nrf_delay_us(us);
}


void rtc_init() { }

static struct tm tm;
struct tm *hw_get_time(void) { return &tm; }
