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
SDK is available at <https://developer.pebble.com/sdk/download/>.
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
    pebble ping --emulator aplite
```
