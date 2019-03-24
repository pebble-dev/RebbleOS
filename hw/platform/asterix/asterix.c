/* asterix.c
 * boot routines for aWatch2
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "nrf_delay.h"

// extern void *strcpy(char *a2, const char *a1);

/*** debug routines ***/

void debug_init() {
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len) {
}

/*** platform ***/

void platform_init() {
}

void platform_init_late() {
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

void hw_backlight_init() { }

void hw_backlight_set() { }

void delay_us(int us) {
    nrf_delay_us(us);
}

void hw_display_init() { }
void hw_display_reset() { }
void hw_display_start_frame(uint8_t xoffset, uint8_t yoffset) { }
static uint8_t _display_fb[168][20];
uint8_t *hw_display_get_buffer() {
    return (uint8_t *)_display_fb;
}
uint8_t hw_display_process_isr() {
    return 1;
}

void hw_flash_init() { }
void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) { }

void rtc_init() { }

static struct tm tm;
struct tm *hw_get_time(void) { return &tm; }

uint8_t hw_bluetooth_init() {
    return 0;
}

void bt_device_request_tx() {
}

void hw_button_init(void) {
}

void hw_button_set_isr(hw_button_isr_t isr) {
}

int hw_button_pressed(hw_button_t button_id) {
    return 0;
}
