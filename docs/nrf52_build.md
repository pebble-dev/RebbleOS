# Building for nRF52 devices

To build for nRF52-based devices, you need the
[latest nRF52 SDK from Nordic's
site](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK) (at
least version 16.0.0).
Download and unzip it; note the path to the folder that it unzips to.

Additionally, you will need a newer build toolchain than that which is
supplied with the Pebble SDK.  On Mac OS X, you will want to download the
[latest GNU ARM Embedded Toolchain from ARM's
site](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads);
on Debian, it is sufficient to `apt-get install gcc-arm-none-eabi`.

Finally, to configure the RebbleOS build to use the nRF5 SDK and the newer
toolchain, you need to create a `localconfig.mk`.  On my Linux system, with
the toolchain installed from `apt`, this contains:

    NRF52_SDK_PATH=/home/joshua/pebble/nRF5SDK160098a08e2
    PEBBLE_TOOLCHAIN_PATH=/usr/bin

On my Mac, this contains:

    NRF52_SDK_PATH=/Users/joshua/pebble-dev/hw/nRF5SDK160098a08e2
    PEBBLE_TOOLCHAIN_PATH=/Users/joshua/pebble-dev/gcc-arm-none-eabi-7-2018-q2-update/bin

You can then build for Asterix-family devices by running `make asterix` or
`make asterix_vla_dvb1`.

Here are commands to do some of these things:

    cd ~/pebble-dev
    wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK160098a08e2.zip
    mkdir nRF5SDK160098a08e2; cd nRF5SDK160098a08e2
    unzip ../nRF5SDK160098a08e2.zip
    cd ..
    git clone https://github.com/pebble-dev/RebbleOS
    cd RebbleOS
    git checkout asterix
    echo "NRF52_SDK_PATH=$HOME/pebble-dev/nRF5SDK160098a08e2" > localconfig.mk
    echo "PEBBLE_TOOLCHAIN_PATH=/usr/bin" >> localconfig.mk
    git submodule update --init --recursive
    make asterix

# Getting started on Asterix-family devices

Booting RebbleOS on Asterix-family devices takes some doing to get initially
set up, but the workflow is not too extreme.  To get hardware set up,
connect SWD to your board -- ideally, using a ST-Link, but any
OpenOCD-supported SWD dongle will work -- and also connect a UART.  (On
`asterix_vla_dvb1`, the UART output is on the test point for P0.29; I have
that connected to a LVTTL-level FTDI dongle.) Make sure you have a
compatible OpenOCD installed; on my machine, I use:

    joshua@bruges:~/pebble-dev/fwdev/FreeRTOS-Pebble$ openocd -v
    Open On-Chip Debugger 0.10.0+dev-00809-g7ee61869 (2019-05-09-12:02)

First, connect the FTDI dongle to your system, and connect to it.  On my
Mac, I use a command like:

    sudo cu -l /dev/tty.usbserial-12345678 -230400

Then, start OpenOCD from inside the `RebbleOS` root directory.  (It's
important which directory you start OpenOCD from the `RebbleOS` directory,
for when you flash the resource pack to the external QSPI flash.)  With an
ST-Link, I use a command like:

    openocd -f interface/stlink-v2.cfg -f target/nrf52.cfg

If OpenOCD starts successfully, it should say something like:

    Info : Listening on port 6666 for tcl connections
    Info : Listening on port 4444 for telnet connections
    Info : clock speed 1000 kHz
    Info : STLINK V2J33S7 (API v2) VID:PID 0483:3748
    Info : Target voltage: 3.144773
    Info : nrf52.cpu: hardware has 6 breakpoints, 4 watchpoints
    Info : Listening on port 3333 for gdb connections

In a new terminal, connect to OpenOCD:

    joshua@bruges:~/pebble-dev/fwdev/FreeRTOS-Pebble$ nc localhost 4444
    Open On-Chip Debugger
    >

You're now ready to flash the microcontroller.  (If you haven't built a
binary for your platform, do that now with a command like `make
asterix_vla_dvb1`.) First flash the softdevice, then flash the system image:

    > reset halt
    reset halt
    target halted due to debug-request, current mode: Thread
    xPSR: 0x01000000 pc: 0x00000998 msp: 0x20000400
    > program /PATH/TO/nRF5_SDK/components/softdevice/s140/hex/s140_nrf52_7.0.1_softdevice.hex
    program /PATH/TO/nRF5_SDK/components/softdevice/s140/hex/s140_nrf52_7.0.1_softdevice.hex
    [...]
    wrote 155648 bytes from file /PATH/TO/nRF5_SDK/components/softdevice/s140/hex/s140_nrf52_6.1.0_softdevice.hex in 6.273110s (24.230 KiB/s)
    ** Programming Finished **
    > flash write_image erase build/asterix_vla_dvb1/tintin_fw.bin 0x27000
    flash write_image erase build/asterix_vla_dvb1/tintin_fw.bin 0x27000
    [...]
    wrote 335872 bytes from file build/asterix_vla_dvb1/tintin_fw.bin in 13.415920s (24.449 KiB/s)
    >

Now that the microcontroller has been programmed, connect GDB:

    $ /your/toolchain/path/arm-none-eabi-gdb build/asterix_vla_dvb1/tintin_fw.elf -ex 'target remote localhost:3333'
    GNU gdb (GNU Tools for Arm Embedded Processors 7-2018-q2-update) 8.1.0.20180315-git
    Copyright (C) 2018 Free Software Foundation, Inc.
    [...]
    Reading symbols from build/asterix_vla_dvb1/tintin_fw.elf...done.
    Remote debugging using localhost:3333
    0x00000998 in ?? ()
    (gdb)

To do the backdoor resource pack load, set a breakpoint before the system
boots (to ensure that other flash operations don't get in the way), turn on
ARM semihosting, reset the system, and run the special backdoor load script. 
ARM semihosting allows firmware on the device to make syscalls through
OpenOCD, including reading files from OpenOCD's path; this is why it is
important that OpenOCD is launched from the correct directory.  (The
resource pack only needs to be loaded once -- or when it is changed!  The
resource pack is stored on the QSPI flash, instead of on the
microcontroller, which is why it is flashed separately.)

    (gdb) monitor reset halt
    target halted due to debug-request, current mode: Thread
    xPSR: 0x01000000 pc: 0x00000998 msp: 0x20000400, semihosting
    (gdb) monitor arm semihosting on
    semihosting is enabled
    (gdb) break hw_flash_init
    Breakpoint 1 at 0x67566: file hw/drivers/nrf52_qspi_flash/nrf52_qspi_flash.c, line 37.
    (gdb) c
    Continuing.
    Note: automatically using hardware breakpoints for read-only addresses.
    
    Breakpoint 1, hw_flash_init () at hw/drivers/nrf52_qspi_flash/nrf52_qspi_flash.c:37
    37          nrfx_qspi_config_t config = NRFX_QSPI_DEFAULT_CONFIG;
    (gdb) finish
    Run till exit from #0  hw_flash_init () at hw/drivers/nrf52_qspi_flash/nrf52_qspi_flash.c:37
    halted: PC: 0x00067568
    flash_init () at rcore/flash.c:26
    26          _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    (gdb) print hw_flash_backdoor_load_respack()
    $1 = void

While this runs, you should get debugging messages on the serial console,
along the lines of:

    *** Taking over the system in semihosting mode to write data -- hold on tight!
    Step 0: opening build/asterix/res/asterix_res.pbpack...
    ... fd 11
    Step 1: erasing region...
    ................Step 2: writing to flash...
    ...0...
    ...16384...
    [...]
    ...163840...
    ...done; wrote 177381 bytes

Finally, delete the breakpoint, and boot the system.

    (gdb) delete
    Delete all breakpoints? (y or n) y
    (gdb) monitor reset halt
    target halted due to debug-request, current mode: Thread
    xPSR: 0x01000000 pc: 0x00000998 msp: 0x20000400, semihosting
    (gdb) c
    Continuing.

Your Asterix system should now be running RebbleOS!
