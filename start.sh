#!/bin/sh
echo 'starting QEMU for Pebble Time (439)'
#qemu-pebble -rtc base=localtime -serial null -serial null -serial stdio -gdb tcp::63770,server -machine pebble-snowy-bb -cpu cortex-m4 -pflash fw.qemu_flash.bin -pflash PebbleImages/qemu_spi_flash.bin
#qemu-system-arm -rtc base=localtime -serial null -serial tcp::63775,server,nowait -serial stdio -gdb tcp::63770,server -machine pebble-snowy-bb -cpu cortex-m4 -pflash fw.qemu_flash.bin -pflash PebbleImages/qemu_spi_flash.bin
qemu-system-arm -rtc base=localtime -serial null -serial null -serial stdio -serial null -serial null -serial null -serial tcp::63767,server,nowait -serial tcp::63768,server,nowait -gdb tcp::63770,server -machine pebble-snowy-bb -cpu cortex-m4 -pflash fw.qemu_flash.bin -pflash Resources/qemu_spi_flash.bin
