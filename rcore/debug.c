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

__attribute__((__noreturn__)) static void _panic(const char *s) {
    in_panic = 1;
    portDISABLE_INTERRUPTS();
    puts("*** PANIC ***");
    puts(s);
    while (1)
        ;
    /* XXX: do something smarter here, like turn IRQs off and stop poking the watchdog */
}

 __attribute__((__noreturn__))void panic(const char *s) {
    asm volatile(
        "mov sp, %[stacktop]\n"
        "mov r0, %[s]\n"
        "b _panic" :
        :
        [stacktop]"r" (_panic_stack + PANIC_STACK_SIZE),
        [s]"r" (s) );
    __builtin_unreachable();
}
