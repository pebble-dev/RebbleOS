A FreeRTOS implementation for the Pebb Time board (STM32F439)

Also comes with a bootloader ripped from quemu images.


Scripts attached to rip a bootloader for the device used

 you must have the path in the PATH variable that includes the pebble sdk
PATH=$PATH:/home/baz/Pebble/SDK/pebble-sdk-4.5-linux64/bin;$PATH

run makeit.sh to build (or run make)


Barry Carter
<barry.carter@gmail.com>




original prject readme below


# STM32F4-FreeRTOS

A demo project of FreeRTOS running on a STM32F4 Discovery board.

## Steps to run this example

### Prerequisite

1. A PC running Linux or Windows with Cygwin(not tested). A Mac is also fine with this example.
2. A STM32F4Discovery board.
3. A FT232RL USB to serial board which is recommended if there's no serial port on your computer.
4. USB Cable, Dupont Line and other tools.

### Install the toolchain

The pre-built version of GNU Tools for ARM can be downloaded from its [website](https://launchpad.net/gcc-arm-embedded). It's available for most systems. Follow the instructions in the readme file and installed the toolchain to your system. To verify your installation, simply type `arm-none-eabi-gcc --version` in your terminal, if everything goes right, you'll get output like this:

```
arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.7.3 20130312 (release) [ARM/embedded-4_7-branch revision 196615]
Copyright (C) 2012 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Install ST-Link utility

#### Windows
Grab the official utility from [ST website](http://www.st.com/web/catalog/tools/FM146/CL1984/SC724/SS1677/PF251168). Note that you should install the USB driver before install the st-util.

#### Linux and OS X
Clone this [git](https://github.com/texane/stlink), follow the instructions on that page and install st-util to your system.

### Compile this example
The only thing you need to do is to edit the makefile and let it know your toolchain installation path. Change the `TOOLCHARN_ROOT` variable at the third line of makefile and point it to where you installed the toolchain. The you can simply type `make` and compile the example.

### Debug
Connect your STM32F4Discovery with a USB cable. You can flash the binary into the board with this:

`$ st-flash write binary/FreeRTOS.bin 0x8000000`

The code is wrote directly into internal flash of STM32 processor and it starts to run after reset. To debug it, first start the GDB server:

`$ st-util &`

And then GDB:

```
$ arm-none-eabi-gdb binary/FreeRTOS.elf
(gdb) tar ext :4242
(gdb) b main
(gdb) c
```

You'll get breakpoint triggered at `main` function, and enjoy!
