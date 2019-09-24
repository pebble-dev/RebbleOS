# Building on macOS

The following builds RebbleOS on macOS:

    git clone https://github.com/pebble-dev/RebbleOS.git
    cd RebbleOS
    git submodule update --init --recursive
    make

The Pebble SDK is a prerequisite for portions of RebbleOS. The
SDK is available at <https://developer.pebble.com/sdk/download/>.
On macOS you can install the SDK with `brew`

    brew install pebble/pebble-sdk/pebble-sdk

If you don't use `brew` (or prefer not to) you can [install the SDK manually](https://developer.pebble.com/sdk/install/mac/).
