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
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "portmacro.h"

static StaticSemaphore_t _dprint_lock_buf;
static SemaphoreHandle_t _dprint_lock = NULL;

static int _lock() {
	if (!_dprint_lock) {
		_dprint_lock = xSemaphoreCreateBinaryStatic(&_dprint_lock_buf);
		xSemaphoreGive(_dprint_lock);
	}
	
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return 1;
	
	if (xPortIsInsideInterrupt()) {
		BaseType_t dmy;
		
		if (!xSemaphoreTakeFromISR(_dprint_lock, &dmy)) {
			return 0;
		}
		
		return 1;
	}
	
	xSemaphoreTake(_dprint_lock, portMAX_DELAY);
	
	return 1;
}

static void _unlock() {
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return;
	
	if (xPortIsInsideInterrupt()) {
		BaseType_t dmy;
		
		xSemaphoreGiveFromISR(_dprint_lock, &dmy);
		
		return;
	}
	
	xSemaphoreGive(_dprint_lock);
}

int putchar(int c) {
	unsigned char _c = c;
	
	if (!_lock())
		return c;
	
	debug_write(&_c, 1);
	
	_unlock();
	
	return c;
}

int puts(const char *s) {
	if (!_lock())
		return 0;
	
	debug_write((const unsigned char *)s, strlen(s));
	
	_unlock();
	
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

