#!/bin/bash


BREW_SDK="$HOME/Library/Application Support/Pebble SDK/SDKs/current/sdk-core/pebble"
if hash brew 2>/dev/null;then
	BREW=1
fi

if [ $# -eq 1 ];then
	SDK=$1
else 
	if [ ! -z "$BREW" ];then
		SDK=$BREW_SDK
	else
		echo "usage: $0 SDK-path"
		exit 1
	fi
fi

echo "Extracting resources from $SDK..."
if [ ! -f "$SDK/aplite/qemu/qemu_micro_flash.bin" ]; then
	echo "$SDK/aplite/qemu/qemu_micro_flash.bin doesn't look like an SDK path to me, buster"
	exit 1
fi

mkdir -p Resources

if [ -f Resources/snowy_boot.bin ]; then
	echo "snowy_boot already exists, not overwriting"
else
	dd if="$SDK/basalt/qemu/qemu_micro_flash.bin" of=Resources/snowy_boot.bin bs=16384 count=1 
fi

if [ -f Resources/snowy_spi.bin ]; then
	echo "snowy_spi already exists, not overwriting"
else
	if [ -f "$SDK/basalt/qemu/qemu_spi_flash.bin.bz2" ];then
		bzip2 -d "$SDK/basalt/qemu/qemu_spi_flash.bin.bz2"
	fi
	cp "$SDK/basalt/qemu/qemu_spi_flash.bin" "Resources/snowy_spi.bin"
fi

if [ -f Resources/tintin_boot.bin ]; then
	echo "tintin_boot already exists, not overwriting"
else
	dd if="$SDK/aplite/qemu/qemu_micro_flash.bin" of=Resources/tintin_boot.bin bs=16384 count=1
fi

if [ -f Resources/tintin_spi.bin ]; then
	echo "tintin_spi already exists, not overwriting"
else
	if [ -f "$SDK/aplite/qemu/qemu_spi_flash.bin.bz2" ];then
		bzip2 -d "$SDK/aplite/qemu/qemu_spi_flash.bin.bz2"
	fi
	cp "$SDK/aplite/qemu/qemu_spi_flash.bin" "Resources/tintin_spi.bin"
fi

if [ -f Resources/chalk_boot.bin ]; then
echo "tintin_boot already exists, not overwriting"
else
dd if="$SDK/chalk/qemu/qemu_micro_flash.bin" of=Resources/chalk_boot.bin bs=16384 count=1
fi

if [ -f Resources/chalk_spi.bin ]; then
echo "tintin_spi already exists, not overwriting"
else
if [ -f "$SDK/chalk/qemu/qemu_spi_flash.bin.bz2" ];then
bzip2 -d "$SDK/chalk/qemu/qemu_spi_flash.bin.bz2"
fi
cp "$SDK/chalk/qemu/qemu_spi_flash.bin" "Resources/chalk_spi.bin"
fi

if [ -f Resources/silk_boot.bin ]; then
echo "silk_boot already exists, not overwriting"
else
dd if="$SDK/diorite/qemu/qemu_micro_flash.bin" of=Resources/silk_boot.bin bs=16384 count=1
fi

if [ -f Resources/silk_spi.bin ]; then
echo "silk_spi already exists, not overwriting"
else
if [ -f "$SDK/diorite/qemu/qemu_spi_flash.bin.bz2" ];then
bzip2 -d "$SDK/diorite/qemu/qemu_spi_flash.bin.bz2"
fi
cp "$SDK/diorite/qemu/qemu_spi_flash.bin" "Resources/silk_spi.bin"
fi


echo "all done"
echo "you may have to execute 'cp Resources/* ../Resources/"
