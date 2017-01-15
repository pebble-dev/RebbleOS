#!/bin/sh
#dd if=PebbleImages/qemu_micro_flash.bin of=PebbleImages/snowy_boot.bin count=16384 bs=1
# turn the flash FPGA object into a gcc o file
arm-none-eabi-objcopy --rename-section .data=.rodata,contents,alloc,load,readonly,data -I binary -O elf32-littlearm -B arm7 PebbleImages/FPGA_4.3_snowy_dumped.bin PebbleImages/FPGA_4.3_snowy.o

