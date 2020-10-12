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

void ghost(uint32_t err) {
	extern const uint8_t rghost_bw[];
	extern const uint8_t font[];
	
	printf("GHOST: %08x\n", err);

	uint8_t *dispbuf = hw_display_get_buffer();
	for (int y = 0; y < DISPLAY_ROWS; y++)
		memcpy(dispbuf + y * 20, rghost_bw + y * 18, 18);
	hw_display_start_frame(0, 0);
	
	for (int c = 0; c < 8; c++) {
		uint8_t nybble = (err >> ((7 - c) * 4)) & 0xF;
		
		for (int y = 0; y < 8; y++)
			dispbuf[20 * (140 + y) + 5 + c] = font[nybble * 8 + y];
	}
	
	while(1)
		;
}

void main() {
	platform_init();
	debug_init();
	hw_watchdog_init();
	hw_watchdog_reset();
	
	printf("RebbleOS bootloader\n\n");
	
	hw_display_init();

	ghost(0xEA80E0B3);
	
	while(1)
		;
}
