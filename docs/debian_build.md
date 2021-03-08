# Building on Debian (Buster)

## Prepare the development environment

Create a directory for the development environment and cd in to it.

```sh
mkdir ~/pebble-dev
cd ~/pebble-dev
```


## Install Dependencies

This will install python, node, gawk and a couple of required libraries

```sh
curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
sudo apt install nodejs python-pip python3-pip python2.7-dev python-gevent gawk libsdl1.2debian libfdt1 libpixman-1-0
```

We're also going to need a cross compiler

```sh
wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
tar jxf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
rm gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2

cd gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux
sudo cp -R * /usr/
cd ~/pebble-dev
rm -rf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux
```

To make sure the cross compiler is working run

```sh
arm-none-eabi-gcc --version
```

The output should look like:

```
arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10-2020-q4-major) 10.2.1 20201103 (release)
Copyright (C) 2020 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

## PebbleSDK

Download and extract the pebble sdk.

```sh
wget https://developer.rebble.io/s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.5-linux64.tar.bz2
tar -jxf pebble-sdk-4.5-linux64.tar.bz2
rm pebble-sdk-4.5-linux64.tar.bz2
```

Add the PebbleSDK binaries to `PATH` and apply the change.

```sh
echo 'export PATH=~/pebble-dev/pebble-sdk-4.5-linux64/bin:$PATH' >> ~/.profile
. ~/.profile
```

Install virtualenv.

```sh
pip3 install virtualenv
```

Install PebbleSDK.

```sh
cd ~/pebble-dev/pebble-sdk-4.5-linux64
virtualenv --python=python2.7 .env
source .env/bin/activate
pip install -r requirements.txt
deactivate
```

Deactivate PebbleSDK analytics tracking.

```sh
mkdir -p ~/.pebble-sdk
touch ~/.pebble-sdk/NO_TRACKING
```

Install the PebbleSDK core.

```sh
pebble sdk install https://github.com/aveao/PebbleArchive/raw/master/SDKCores/sdk-core-4.3.tar.bz2
```

To test if everything was successful, you can run:

```sh
pebble ping --emulator aplite
```

> **_NOTE:_**  The emulator requires an active X server.


## RebbleOS

Navigate back to our development environment.

```sh
cd ~/pebble-dev
```

Make sure `git` is installed.

```sh
sudo apt install git
```

Clone the RebbleOS repository, and install its dependencies.

```sh
git clone https://github.com/pebble-dev/RebbleOS.git
cd RebbleOS
git submodule update --init --recursive
```

Create a `Resources` folder.

```sh
mkdir Resources
```

At this point, you will need to visit the [Discord](http://discord.gg/aRUAYFN) to download the FPGA files and put them in the `Resources` folder.
Which can be found in the #firmware channel in the Pins.

Copy additional resources from the PebbleSDK using a utility script.

```sh
./Utilities/mk_resources.sh ~/.pebble-sdk/SDKs/4.3/sdk-core/pebble
```

Now we need to make a small change to `lib/tz/Makefile` to use `gawk` instead of `awk`

Open the file in your favorite text editor and find the code that says:

```
# The name of a Posix-compliant 'awk' on your system.
# Older 'mawk' versions, such as the 'mawk' in Ubuntu 16.04, might dump core;
# on Ubuntu you can work around this with
#       AWK=            gawk
AWK=            awk
```

Change `AWK=awk` to `AWK=gawk` (the whitespace in between is not important, you can leave it or remove it). Save and exit the editor

It should now be ready to compile and run in the emulator

```sh
cd ~/pebble-dev/RebbleOS
make snowy_qemu
```


If anything fails, or if you have any questions, feel free to hit us up in [Discord](http://discord.gg/aRUAYFN)!
