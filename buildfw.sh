#!/bin/sh

PEBBLE_IMAGE_PATH=~/Pebble/Pebble-4.3-snowy_dvt
PEBBLE_FIRMWARE_UTILS=~/Pebble/pebble-firmware-utils-master

cp build/snowy/tintin_fw.bin $PEBBLE_IMAGE_PATH/tintin_fw.bin

sh -c "cd $PEBBLE_IMAGE_PATH && python $PEBBLE_FIRMWARE_UTILS/repackFirmware.py pebble.pbz"
cp -rfv $PEBBLE_IMAGE_PATH/pebble.pbz ./binary/
curl -T ./binary/pebble.pbz ftp://192.168.0.104:2121 --user user:pass
