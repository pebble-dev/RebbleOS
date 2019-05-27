/*
 * fault_handlers.c
 * fault handlers for nRF52 boards
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "debug.h"
#include "rebbleos.h"

struct {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11, r13;
} fault_regs;

void HardFault_Handler_C(uint32_t *msp, uint32_t *psp, uint32_t lr)
{
    int is_psp = lr & 4;
    int is_thread = lr & 8;
    uint32_t *sp = is_psp ? psp : msp;
    
    printf("*** HARD FAULT: on %s, in %s mode ***\n", is_psp ? "PSP" : "MSP", is_thread ? "thread" : "handler");
    printf("   R0: %08lx,  R1: %08lx,  R2: %08lx,  R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("   R4: %08lx,  R5: %08lx,  R6: %08lx,  R7: %08lx\n", fault_regs.r4, fault_regs.r5, fault_regs.r6, fault_regs.r7);
    printf("   R8: %08lx,  R9: %08lx, R10: %08lx, R11: %08lx\n", fault_regs.r8, fault_regs.r9, fault_regs.r10, fault_regs.r11);
    printf("  R12: %08lx, R13: %08lx,  LR: %08lx,  PC: %08lx\n", sp[4], fault_regs.r13, sp[5], sp[6]);
    printf("  MSP: %08lx, PSP: %08lx, EXC: %08lx\n", (uint32_t)msp, (uint32_t)psp, lr);
    while(1);
}

__attribute__((naked)) void HardFault_Handler()
{
    asm volatile (
        "ldr r0, =fault_regs\n"
        "str r4, [r0, #0]\n"
        "str r5, [r0, #4]\n"
        "str r6, [r0, #8]\n"
        "str r7, [r0, #12]\n"
        "str r8, [r0, #16]\n"
        "str r9, [r0, #20]\n"
        "str r10, [r0, #24]\n"
        "str r11, [r0, #28]\n"
        "str r13, [r0, #32]\n"
        "mrs r0, msp\n"
        "mrs r1, psp\n"
        "mov r2, lr\n"
        "ldr r3, =HardFault_Handler_C\n"
        "bx  r3\n"
    );
}

void BusFault_Handler()
{
    printf("*** BUS FAULT ***\n");
    while(1);
}

void UsageFault_Handler_C(uint32_t *sp)
{
    uint16_t ufsr = *(uint16_t *)0xE000ED2A;
    
    printf("*** USAGE FAULT ***\n");
    printf("   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx\n", sp[4], sp[5], sp[6], (uint32_t) sp);
    printf("  UFSR: %04x\n", ufsr);
    
    if (ufsr & 1) {
        printf("    *PC == %04x\n", *(uint16_t *)sp[6]);
    }
    while(1);
}

__attribute__((naked)) void UsageFault_Handler()
{
    asm volatile (
        "TST   LR, #4\n\t"
        "ITE   EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"
        "LDR   R1, =UsageFault_Handler_C\n\t"
        "BX    R1"
    );
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    printf("*** error %ld (pc %lx, info %lx)\n", id, pc, info);
    panic("app_error_fault_handler from nRF SDK");
}

void app_error_handler_bare(ret_code_t error_code) 
{
    app_error_fault_handler(error_code, 0, 0);
}
