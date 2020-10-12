#include "minilib.h"
#include <stdarg.h>
#include "platform.h"
#include "core_cm4.h"
#include "nrf_sdm.h"

extern int printf(const char *, ...);
extern int vprintf(const char *ifmt, va_list ap);
extern void delay_us(uint32_t us);

#define SECONDARY_START 0x30000

volatile int display_is_done = 0;
void display_done_isr(int cmd) {
	display_is_done = hw_display_process_isr();
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
	
	for (int c = 0; c < 8; c++) {
		uint8_t nybble = (err >> ((7 - c) * 4)) & 0xF;
		
		for (int y = 0; y < 8; y++)
			dispbuf[20 * (140 + y) + 5 + c] = font[nybble * 8 + y];
	}

	display_is_done = 0;
	hw_display_start_frame(0, 0);
	while (!display_is_done)
		;
}

void main() {
	platform_init();
	debug_init();
	hw_watchdog_init();
	hw_watchdog_reset();
	
	printf("\nRebbleOS bootloader\n\n");
	
	hw_display_init();

	ghost(0xEA80E0B3);
	
	printf("Booting OS:\n");
	printf("  stack top = %08lx\n", *(uint32_t *)0x30000);
	printf("  reset = %08lx\n", *(uint32_t *)0x30004);
	
	/* XXX: Asterix- and nRF52-specific right now. */
	sd_softdevice_vector_table_base_set(0x30000);
	
	asm volatile(
	"mov sp, %0\n"
	"bx %1\n"
	: : "r"(*(uint32_t *)0x30000), "r"(*(uint32_t *)0x30004)
	);
	
	while(1)
		;
}
