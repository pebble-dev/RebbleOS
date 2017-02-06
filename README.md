A FreeRTOS implementation for the Pebble Time board (STM32F439)

Stage 1 Goals

* To create a new, unique operating system for the Pebble hardware.
  - Reverse engineer and re-implement each piece of hardware on the device
  - That includes the bluetooth stack!
  - Create a new set of application APIs
  - Obtain full integration with Rebble.io store
* Re-implement all watchface functions either completely, or as stubs
* Implement a new ecosystem of app/watchface development


Stage 2 Goals (long ways off)

* Re-implement as much as possible of the Pebble api set as possible
* Attempt to attain binary compatibility for apps and watchfaces
* i8n


run makeit.sh to build (or run make)

Working:
* CPU core / peripherals.
* Interrupts
* Buttons on PT (currently interrupt driven)
* Display backlight fade (stays on for n millis and then fades out)
* Vibrate (no control path yet)
* Basic charge information
* Real Time Clock
* Watchdog
* Smart Strap for debugging output
* Display!
  - Shows a Rebble OS splash (Thanks for the convert @XDJackieXD)
  - Base support for text and graphics output using uGUI
  - Display almost fully functional
  - 


It will come

Hardware TODO:
* Gyro and Compass
* More power management
  - We need to turn clocks off when not in use etc
* Flash memory
* Microphone
* Find the gpio for smart strap power

Instructions to compile:

you must have the path in the PATH variable that includes the pebble sdk
PATH=$PATH:/home/baz/Pebble/SDK/pebble-sdk-4.5-linux64/bin;$PATH

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

