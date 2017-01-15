A FreeRTOS implementation for the Pebble Time board (STM32F439)

Also comes with a bootloader ripped from qemu images.


Scripts attached to rip a bootloader for the device used

you must have the path in the PATH variable that includes the pebble sdk
PATH=$PATH:/home/baz/Pebble/SDK/pebble-sdk-4.5-linux64/bin;$PATH

run makeit.sh to build (or run make)

Working:
* CPU core / peripherals.
* Interruts
* Buttons on PT
* Display backlight fade
* Vibrate
* Basic charge information
* Real Time Clock
* Watchdog
* Display!
  - Shows a checkerboard on boot for now

It will come

Hardware TODO:
Split the RTOS away from the hardware implementation ala snowy_display ready for multiple platform support
Gyro and Compass
More power management
Flash memory
Microphone
Light sensor

Barry Carter
<barry.carter@gmail.com>

