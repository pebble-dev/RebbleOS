#!/bin/bash
MAP=$1

getsym() {
	grep " $1 =" $MAP | awk '{ print $1 }'
}

FLASH_REMAIN=$(($(getsym _flash_top) - ($(getsym _edata) - $(getsym _sdata) + $(getsym _erodata))))
RAM_REMAIN=$(($(getsym _ram_top) - $(getsym _end)))

echo "$RAM_REMAIN bytes of RAM available for heap."
echo "$FLASH_REMAIN bytes of flash unused."
