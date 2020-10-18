/* bootbits.c
 * Software implementation of reset-persistent bootbits for nRF52840
 * RebbleOS
 *
 * nRF52840 doesn't have RTC registers that are set to a known value on POR,
 * which is how PebbleOS normally stashes a word of data to communicate
 * between the bootloader and the OS.  So we set up the linker script to
 * reserve the upper 256 bytes of RAM for our bootbits, and we carefully
 * check to see if the bootbits have been initialized (i.e., if our guard
 * value has been written) before we trust them.
 */

#include "platform_bootbits.h"

#define BOOTBITS_GUARD_VALUE 0xAA55AA55

static uint32_t _bootbits_guard __attribute__ ((section(".bootbits")));
static uint32_t _bootbits __attribute__ ((section(".bootbits")));

static void _check_bootbits() {
	if (_bootbits_guard != BOOTBITS_GUARD_VALUE) {
		_bootbits_guard = BOOTBITS_GUARD_VALUE;
		_bootbits = 0;
	}
}

int hw_bootbits_test(uint32_t bootbit) {
	_check_bootbits();
	return _bootbits & bootbit;
}

void hw_bootbits_clear(uint32_t bootbit) {
	_check_bootbits();
	_bootbits &= ~bootbit;
}

void hw_bootbits_set(uint32_t bootbit) {
	_check_bootbits();
	_bootbits |= bootbit;
}
