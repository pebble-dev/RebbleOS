# Building on Debian Stretch

The following builds RebbleOS on Debian Stretch:

```sh
    apt install -y gcc-arm-none-eabi
    git clone https://github.com/ginge/FreeRTOS-Pebble.git
    cd FreeRTOS-Pebble
    git submodule update --init --recursive
    make
```

The Pebble SDK is a prerequisite for portions of RebbleOS. The
SDK is available [here](https://developer.rebble.io/developer.pebble.com/sdk/download/index.html).
As an example, on Debian, one may create an installation
directory for the SDK using the following.

```sh
    mkdir ~/pebble-dev/
    cd ~/pebble-dev/
    wget https://s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.5-linux64.tar.bz2
    tar -jxf pebble-sdk-4.5-linux64.tar.bz2
    echo 'export PATH=~/pebble-dev/pebble-sdk-4.5-linux64/bin:$PATH' >> ~/.bash_profile
    . ~/.bash_profile
    sudo apt-get update
    sudo apt-get install python-pip python2.7-dev python-gevent libsdl1.2debian libfdt1 libpixman-1-0 git gcc-arm-none-eabi npm
    pip install virtualenv
    pip install --upgrade pip
    cd ~/pebble-dev/pebble-sdk-4.5-linux64
    virtualenv --no-site-packages .env
    source .env/bin/activate
    pip install -r requirements.txt
    deactivate
    mkdir ~/.pebble-sdk/
    touch ~/.pebble-sdk/NO_TRACKING
    pebble sdk install https://github.com/aveao/PebbleArchive/raw/master/SDKCores/sdk-core-4.3.tar.bz2
    pebble ping --emulator aplite
```

As the SDK is no longer available from an official source, this example uses an archived version from [this](https://github.com/aveao/PebbleArchive/) GitHub repository
