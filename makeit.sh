#!/bin/sh
make -j4
./create_image.sh

~/Pebble/pebble-firmware-utils-master/repackFirmware.py 
