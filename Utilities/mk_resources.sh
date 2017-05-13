#!/bin/bash

if [ $# -ne 1 ]; then
	echo "usage: $0 SDK-path"
	exit 1
fi

SDK=$1
echo "Extracting resources from $SDK..."
if [ ! -f $SDK/Pebble/aplite/qemu/qemu_micro_flash.bin ]; then
	echo "$SDK doesn't look like an SDK path to me, buster"
	exit 1
fi

mkdir -p Resources

if [ -f Resources/snowy_boot.bin ]; then
	echo "snowy_boot already exists, not overwriting"
else
	dd if=$SDK/Pebble/basalt/qemu/qemu_micro_flash.bin of=Resources/snowy_boot.bin bs=16384 count=1 
fi

if [ -f Resources/snowy_spi.bin ]; then
	echo "snowy_spi already exists, not overwriting"
else
	cp $SDK/Pebble/basalt/qemu/qemu_spi_flash.bin Resources/snowy_spi.bin
fi

if [ -f Resources/tintin_boot.bin ]; then
	echo "tintin_boot already exists, not overwriting"
else
	dd if=$SDK/Pebble/aplite/qemu/qemu_micro_flash.bin of=Resources/tintin_boot.bin bs=16384 count=1
fi

if [ -f Resources/tintin_spi.bin ]; then
	echo "tintin_spi already exists, not overwriting"
else
	cp $SDK/Pebble/aplite/qemu/qemu_spi_flash.bin Resources/tintin_spi.bin
fi

echo "all done"