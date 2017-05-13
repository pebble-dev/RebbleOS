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

int snprintf(char *buf, unsigned int len, const char *ifmt, ...) {
	va_list ap;
	int n;
	
	va_start(ap, ifmt);
	n = vsfmt(buf, len, ifmt, ap);
	va_end(ap);
	
	return n;
}

static void _fmtout(void *p, char c) {
	putchar(c);
}

int printf(const char *ifmt, ...) {
	va_list ap;
	struct fmtctx ctx;
	int num_written;

	ctx.str = ifmt;
	ctx.out = _fmtout;
	ctx.priv = NULL;

	va_start(ap, ifmt);
	num_written = fmt(&ctx, ap);
	va_end(ap);

	return num_written;
}

int vprintf(const char *ifmt, va_list ap) {
	struct fmtctx ctx;
	int num_written;

	ctx.str = ifmt;
	ctx.out = _fmtout;
	ctx.priv = NULL;

	num_written = fmt(&ctx, ap);

	return num_written;
}

