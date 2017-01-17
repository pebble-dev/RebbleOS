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
* Smart Strap for debugging output
* Display!
  - Shows a Rebble OS splash (Thanks for the convert @XDJackieXD)
  - Base support for text and graphics output using uGUI
  - Scanline converter in and mostly working

It will come

Hardware TODO:
* Split the RTOS away from the hardware implementation ala snowy_display ready for multiple platform support
* Gyro and Compass
* More power management
  - We need to turn clocks off when not in use etc
* Flash memory
* Microphone
* Light sensor
* Find the gpio for smart strap power

Instructions to compile:

You will need to get the following packages:

 * Pebble Firmware Utils https://github.com/MarSoft/pebble-firmware-utils
 * Pebble firmware itself (link here)
 * (Optional) Pebble Qemu configured for PT
 
 Edit the file buildfw.sh and change out paths and IP
 
 run:
 
 make fpga
 
 make
 
 make upload
 
 To push the firmware, I used an FTP server on my phone, and pushed the firmware to it. 
 
 You can manually copy the pebble.pbz to the phone your way, in which case comment out the curl lines
 
 if you 
 
 make qemu
 
 Then you get a firmware that will work with the Pebble emulator
 

Barry Carter
<barry.carter@gmail.com>

