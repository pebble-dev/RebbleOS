#!/bin/bash
# Push firmware.pbz to the deviceA simple adb push script to get a RebbleOS firmware onto hardware
platform=$1

if [[ -z $1 ]]; then
    echo "Defaulting to Snowy. Please use '$0 platform' if this isn't what you intended"
    platform="snowy"
fi

echo "I'm going to build platform $platform's PBZ, push it to a connected "
echo "android device over ADB, and then invoke the flash prompt on the pebble app"

if [ "$platform" != "tintin" ] && [ "$platform" != "snowy" ] && [ "$platform" != "chalk" ]; then
    echo "I don't know what platform $1 is!, sorry"
    exit 1
fi
make $platform
adb push ./build/$platform/$platform.pbz /sdcard/Download/
adb shell am start -n com.getpebble.android.basalt/com.getpebble.android.main.activity.MainActivity -a android.intent.action.VIEW -d file:///sdcard/Download/$platform.pbz
