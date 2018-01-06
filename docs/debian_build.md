# Building on Debian Stretch

The following builds RebbleOS on Debian Stretch:

    apt install -y gcc-arm-none-eabi
    git clone https://github.com/ginge/FreeRTOS-Pebble.git
    cd FreeRTOS-Pebble
    make

The Pebble SDK is a prerequisite for portions of RebbleOS. The
SDK is available at <https://developer.pebble.com/sdk/download/>.
As an example, on Debian, one may create an installation
directory for the SDK using the following.

    mkdir ~/pebble-dev/
    cd ~/pebble-dev/
    wget https://s3.amazonaws.com/assets.getpebble.com/pebble-tool/pebble-sdk-4.5-linux64.tar.bz2
    tar -jxf pebble-sdk-4.5-linux64.tar.bz2
    echo 'export PATH=~/pebble-dev/pebble-sdk-4.5-linux64/bin:$PATH' >> ~/.bash_profile
    . ~/.bash_profile
    sudo apt-get install python-pip python2.7-dev python-gevent
    sudo pip install virtualenv
    cd ~/pebble-dev/pebble-sdk-4.5-linux64
    virtualenv --no-site-packages .env
    source .env/bin/activate
    pip install -r requirements.txt
    deactivate

The Pebble emulator requires the following libraries.

    sudo apt-get install libsdl1.2debian libfdt1 libpixman-1-0


