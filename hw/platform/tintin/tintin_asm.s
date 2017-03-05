	.cpu cortex-m3
	.syntax unified
	.code 16

	.globl delay_us
delay_us:
	mov r3, #6
	muls r0, r3
1:
	subs r0, #1
	bne 1b
	bx lr
