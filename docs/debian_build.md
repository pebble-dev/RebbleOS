# Building on Debian Stretch

## Prerequisites

Before building RebbleOS you will need the [Pebble SDK](#PebbleSDK) and if you wish to run the firmware in `qemu`, you'll need to make the [resources](#Qemu-Resources)

## Building Rebble OS
The following builds RebbleOS on Debian Stretch:

```sh
    apt install -y gcc-arm-none-eabi
    git clone https://github.com/pebble-dev/RebbleOS.git
    cd RebbleOS
    git submodule update --init --recursive
    make
```
## PebbleSDK

The Pebble SDK is a prerequisite for portions of RebbleOS. The
SDK is available [here](https://developer.rebble.io/developer.pebble.com/sdk/download/index.html).
As an example, on Debian, one may create an installation
directory for the SDK using the following.

```sh
    mkdir ~/pebble-dev/
    cd ~/pebble-dev/
    wget https://developer.rebble.io/s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.5-linux64.tar.bz2
    tar -jxf pebble-sdk-4.5-linux64.tar.bz2
    echo 'export PATH=~/pebble-dev/pebble-sdk-4.5-linux64/bin:$PATH' >> ~/.bash_profile
    . ~/.bash_profile
    curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
    sudo apt-get update
    sudo apt-get install python-pip python2.7-dev python-gevent libsdl1.2debian libfdt1 libpixman-1-0 git gcc-arm-none-eabi npm
    pip install virtualenv
    pip install --upgrade pip
    cd ~/pebble-dev/pebble-sdk-4.5-linux64
    virtualenv --python=python2.7 .env
    source .env/bin/activate
    pip install -r requirements.txt
    deactivate
    mkdir ~/.pebble-sdk/
    touch ~/.pebble-sdk/NO_TRACKING
    pebble sdk install https://github.com/aveao/PebbleArchive/raw/master/SDKCores/sdk-core-4.3.tar.bz2
    pebble ping --emulator aplite
```

## Qemu Resources

Before building RebbleOS for the first time you will need to run the [Utilities/mk_resources.sh](Utilities/mk_resources.sh) script.

```sh
    cd ~/pebble-dev/RebbleOS/
    mkdir Resources
```

At this point, you will need to visit the [Discord](http://discord.gg/aRUAYFN) and download the FPGA files and place them in the new Resources folder we just made.
Then continue with the steps below.

    
```sh
    cd ~/pebble-dev/RebbleOS/Utilities/
    ./mk_resources.sh ~/.pebble-sdk/SDKs/4.3/sdk-core/pebble
    cp Resources/* ../Resources/
```

That's it you're now ready to build! Head back to the [top](#Building-Rebble-OS)

    
