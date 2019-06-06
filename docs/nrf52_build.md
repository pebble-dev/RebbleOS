# Building for nRF52 devices

To build for nRF52-based devices, you need the
[latest nRF52 SDK from Nordic's
site](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK). 
Download and unzip it; note the path to the folder that it unzips to.

Additionally, you will need a newer build toolchain than that which is
supplied with the Pebble SDK.  On Mac OS X, you will want to download the
[latest GNU ARM Embedded Toolchain from ARM's
site](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads);
on Debian, it is sufficient to `apt-get install gcc-arm-none-eabi`.

Finally, to configure the RebbleOS build to use the nRF5 SDK and the newer
toolchain, you need to create a `localconfig.mk`.  On my Linux system, with
the toolchain installed from `apt`, this contains:

    NRF52_SDK_PATH=/home/joshua/pebble/nRF5_SDK_15.3.0_59ac345
    PEBBLE_TOOLCHAIN_PATH=/usr/bin

On my Mac, this contains:

    NRF52_SDK_PATH=/Users/joshua/pebble-dev/hw/nRF5_SDK_15.2.0_9412b96
    PEBBLE_TOOLCHAIN_PATH=/Users/joshua/pebble-dev/gcc-arm-none-eabi-7-2018-q2-update/bin

You can then build for Asterix-family devices by running `make asterix` or
`make asterix_vla_dvb1`.
