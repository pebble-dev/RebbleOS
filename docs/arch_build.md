# Building on Arch Linux and It's Derivatives

## Prerequisites

Before building RebbleOS you will need the [Pebble SDK](#PebbleSDK) and if you wish to run the firmware in `qemu`, you'll need to make the [resources](#Qemu-Resources)


## Building Rebble OS

```sh
    git clone https://github.com/pebble-dev/RebbleOS.git
    cd RebbleOS
    git submodule update --init --recursive
    cd FreeRTOS
    virtualenv .env
    source .env/bin/activate
    cd ..
    make
    deactivate
```
   
The Pebble SDK is a prerequisite for portions of RebbleOS. The
SDK is available [here](https://developer.rebble.io/developer.pebble.com/sdk/download/index.html).
As an example, on Arch, one may create an installation
directory for the SDK using the following.

```sh
    mkdir ~/pebble-dev/
    cd ~/pebble-dev/
    wget https://developer.rebble.io/s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.5-linux64.tar.bz2
    tar -jxf pebble-sdk-4.5-linux64.tar.bz2
    echo 'export PATH=~/pebble-dev/pebble-sdk-4.5-linux64/bin:$PATH' >> ~/.bash_profile
    . ~/.bash_profile
    sudo pacman -Syyu
    sudo pacman -S python2 python2-pip  python2-gevent sdl dtc pixman git arm-none-eabi-gcc arm-none-eabi-newlib npm
    pip2 install --upgrade pip --user
    pip2 install virtualenv --user
    cd ~/pebble-dev/pebble-sdk-4.5-linux64
    virtualenv .env
    source .env/bin/activate
    pip2 install -r requirements.txt
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

    
