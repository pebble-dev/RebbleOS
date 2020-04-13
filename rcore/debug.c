/* debug.c
 * Debugging facilities, including panic()
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stdio.h>
#include "debug.h"
#include "platform.h"
#include "FreeRTOS.h"

#define PANIC_STACK_SIZE (224 / 2)

int in_panic = 0;

static StackType_t _panic_stack[PANIC_STACK_SIZE] MEM_REGION_PANIC;
static int _panic_stack_top MEM_REGION_PANIC;

__attribute__((__noreturn__)) static void _panic(const char *s) {
    in_panic = 1;
    portDISABLE_INTERRUPTS();
    puts("*** PANIC ***");
    puts(s);
    while (1)
        ;
    /* XXX: do something smarter here, like turn IRQs off and stop poking the watchdog */
}

extern __attribute__((__noreturn__)) void panic(const char *s);
asm(
".global panic\n"
".thumb_func\n"
".type panic, %function\n"
"panic:\n"
"	.cfi_startproc\n"
"	push {r7, lr}\n"
"	.cfi_def_cfa_offset 8\n"
"	.cfi_offset 7, -8\n"
"	.cfi_offset 14, -4\n"
"	mov r7, sp\n"
"	adr r1, _panic_stack_top\n"
"	mov r1, sp\n"
"	.cfi_register 13, 7\n"
/* r0 stays the same -- was s, will continue to be s */
"	bl _panic\n"
"	.cfi_endproc\n" /* "lol" */
);
