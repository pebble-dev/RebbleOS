# RebbleOS

RebbleOS is an open-source reimplementation of the firmware for the devices
formerly manufactured by Pebble Technologies, Inc.  The firmware is based on
FreeRTOS, and aims to be binary-compatible with applications that were
written for the original Pebble OS, as well as radio-compatible with
smartphone applications that are designed to work with Pebble.

## Hacking

RebbleOS needs your help! This section discusses what you need to know to
get started working on the project.

### Building

RebbleOS currently can be built for `snowy` (Pebble Time and Pebble Time
Steel) and `tintin` (Pebble and Pebble Steel).  To build RebbleOS, follow
these steps:

* Obtain a checkout of the RebbleOS source code.
* Create a `localconfig.mk` if your cross-compiler is in an unusual location.  For instance, if you have the SDK installed in `/home/me`, add the following line to your `localconfig.mk`: `PEBBLE_TOOLCHAIN_PATH=/home/me/Pebble/SDK/pebble-sdk-4.5-linux64/arm-cs-tools/bin`.  For more information on `localconfig.mk` variables, consult the `Makefile`.
* Build the firmware: `make`
* If you wish to run the firmware in `qemu`, copy the resources necessary into `Resources/`.  Take a look at [Utilities/mk_resources.sh](Utilities/mk_resources.sh) for more information on that.
* To run the firmware in `qemu`, try `make snowy_qemu`.

[Building on Debian Stretch](docs/debian_build.md)

[Building on macOS](docs/mac_build.md)

If you wish to build firmware to run on your device, you may also wish to
consider a script like [buildfw.sh](buildfw.sh). Running RebbleOS on hardware is
currently out of scope for this document.

> You need the `snowy_fpga.bin` and `chalk_fpga.bin` files to compile on their respective firmwares. They can be found on the `#firmware` channel in the Rebble Discord.

### Code structure

_(This section is, admittedly, somewhat aspirational.  Do not be surprised
if code within RebbleOS does not necessarily conform to this structure
yet!)_

RebbleOS is composed of four major components: the hardware abstraction
layer, the core operating system, the PebbleOS compatibility layer, and
system applications.  We break down these components as follows:

* **Hardware abstraction layer.**  This subsystem provides a unified
  interface for the rest of the system, and abstracts away platform-specific
  elements.  The HAL lives in the directory `hw/`; symbols that the HAL
  exports to the rest of the system are prefixed with `hw_`.  The main
  entity that the HAL works on is a _"platform"_; for an example, take a
  look at [hw/platform/snowy/config.mk](hw/platform/snowy/config.mk).  A platform depends on various chip
  components, and potentially other driver components; it exports a
  [platform.h](hw/platform/snowy/platform.h) that includes all defines that the rest of the system may
  need.  The HAL is, in theory, independent of the rest of the OS; it does
  not call into the rest of the system other than through debugging
  mechanisms and through callbacks that it is provided.

* **Core OS.** This subsystem provides basic services that any smartwatch
  OS, even if not implementing a Pebble-like API, might need.  HAL accesses
  are marshalled through concurrency protection; higher-level power
  management takes place; and, flash wear leveling and filesystem management
  happens in the core OS.  The core OS lives in `rcore/`, and symbols
  exported from the core OS are prefixed with `rcore_`.  It calls on
  FreeRTOS, which lives in `FreeRTOS/`.

* **Pebble compatibility layer.**  The core OS provides basic isolation
  between threads and framebuffer management primitives, but the Pebble
  compatibility layer provides higher level operations, like Pebble-style
  layers, input management and routing, and UI services.  The Pebble
  compatibility layer lives in `rwatch/`, and symbols exported from it are
  prefixed with `rwatch_`.  (Functions that are exactly analogous to Pebble
  APIs may be named with their exact name.)

* **System applications.** We'll, uh, get there when we have some.  Yeah.

## Reuse and contact

RebbleOS is an open-source project licensed (primarily) under a BSD-style
license.  For more information, please see the [LICENSE](LICENSE) and [AUTHORS](AUTHORS)
files.  Additionally, contributors and members of the RebbleOS community are
expected to abide by our code of conduct; for more information on that,
please see the [CODE-OF-CONDUCT.md](CODE-OF-CONDUCT.md) file.  Reuse of this project is not only
permitted, but encouraged!  If you do something cool with RebbleOS, please
get in touch with us.  The easiest way to do so is through the [Rebble
Discord server](https://discordapp.com/invite/aRUAYFN), channel #firmware. 
We look forward to meeting you!
