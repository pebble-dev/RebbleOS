# RebbleOS

RebbleOS is an open-source reimplementation of the firmware for the devices
formerly manufactured by Pebble Technologies, Inc.  The firmware is based on
FreeRTOS, and aims to be binary-compatible with applications that were
written for the original Pebble OS, as well as radio-compatible with
smartphone applications that are designed to work with Pebble.

## Hacking

RebbleOS currently can be built for `snowy` (Pebble Time and Pebble Time
Steel) and `tintin` (Pebble and Pebble Steel).  To build RebbleOS, follow
these steps:

* Obtain a checkout of the RebbleOS source code.
* Create a `localconfig.mk` if your cross-compiler is in an unusual location.  For instance, if you have the SDK installed in `/home/me`, add the following line to your `localconfig.mk`: `PEBBLE_TOOLCHAIN_PATH=/home/me/Pebble/SDK/pebble-sdk-4.5-linux64/arm-cs-tools/bin`.  For more information on `localconfig.mk` variables, consult the `Makefile`.
* Build the firmware: `make`
* If you wish to run the firmware in `qemu`, copy the resources necessary into `Resources/`.  Take a look at `Utilities/mk_resources.sh` for more information on that.
* To run the firmware in `qemu`, try `make snowy_qemu`.

If you wish to build firmware to run on your device, you may also wish to
consider a script like `buildfw.sh`.  Running RebbleOS on hardware is
currently out of scope for this document.

## Reuse and contact

RebbleOS is an open-source project licensed (primarily) under a BSD-style
license.  For more information, please see the `LICENSE` and `AUTHORS`
files.  Reuse of this project is not only permitted, but encouraged!  If you
do something cool with RebbleOS, please get in touch with us.  The easiest
way to do so is through the [Rebble Discord server](https://discordapp.com/invite/aRUAYFN), channel #firmware.  We look forward to meeting you!
