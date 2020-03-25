FROM rebble/nrf52:latest

ADD . /code
WORKDIR /code

# get resources
RUN mkdir -p Resources
RUN Utilities/mk_resources.sh /root/.pebble-sdk/SDKs/4.3/sdk-core/pebble/
RUN curl -ssL http://emarhavil.com/~joshua/snowy_fpga.bin -o Resources/snowy_fpga.bin

RUN echo NRF52_SDK_PATH=/opt/${NRF5_SDK_VERSION} >> localconfig.mk

# set up the python VE
RUN make build/python_env

#default cmd
CMD /bin/bash
