#include "minilib.h"
#include <stdarg.h>
#include "platform.h"

extern int printf(const char *, ...);
extern int vprintf(const char *ifmt, va_list ap);
extern void delay_us(uint32_t us);

void display_done_isr(int cmd) {
	hw_display_process_isr();
}

void flash_operation_complete_isr(int cmd) {
}

void log_printf_to_ar(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...) {
	va_list ap;
	
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	
	printf("\n");
}

void panic(const char *s) {
	printf("PANIC: %s\n", s);
}

void delay_ms(uint32_t ms) {
	delay_us(ms * 1000);
}

void main() {
	platform_init();
	debug_init();
	hw_watchdog_init();
	hw_watchdog_reset();
	
	printf("RebbleOS bootloader\n\n");
	
	while(1)
		;
}
