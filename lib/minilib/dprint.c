/* dprint.c
 * Simple debug-print routines for RebbleOS
 * minilib for RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 *
 * XXX: This write implementation is inherently single-threaded. 
 * Once locking is in place, this should do its own locking.
 */

#include "platform.h"
#include <minilib.h>

int putchar(int c) {
	unsigned char _c = c;
	debug_write(&_c, 1);
	return c;
}

int puts(const char *s) {
	debug_write((const unsigned char *)s, strlen(s));
	putchar('\n');
	return 0;
}
