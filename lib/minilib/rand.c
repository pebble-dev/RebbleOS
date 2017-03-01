/* rand.c
 * Cheesy crc32-derived random number generator
 * minilib for RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 *
 * This rand() implementation is inherently single-threaded.  Any
 * application using this implementation must provide its own locking
 * mechanism.
 */

#include <stdint.h>

#define CRC32_POLY 0x04c11db7 /* AUTODIN II, Ethernet, & FDDI */

static uint32_t _seed = 1;

void srand(uint32_t s) {
	_seed = s;
	if (!s)
		_seed = 1;
}

uint32_t rand() {
	_seed = (_seed & 0x80000000) ? (_seed << 1) ^ CRC32_POLY : (_seed << 1);

	return _seed;
}
