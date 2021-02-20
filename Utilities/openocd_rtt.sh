RTT_ADDR=$(arm-none-eabi-nm build/${1}/tintin_fw.elf | grep '_SEGGER_RTT' | cut -d " " -f1)
openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg -c "init" -c "rtt setup 0x${RTT_ADDR} 2048 \"SEGGER RTT\"" -c "rtt start" -c "rtt server start 9090 0"
