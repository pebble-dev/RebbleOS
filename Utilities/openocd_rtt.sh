RTT_ADDR=$(arm-none-eabi-nm build/${1}/tintin_fw.elf | grep '_SEGGER_RTT' | cut -d " " -f1)
openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg \
-c "proc rtt_init {} {rtt setup 0x${RTT_ADDR} 2048 \"SEGGER RTT\";\
    rtt start;\
    rtt server start 9090 0;}" \
-c "proc rtt_reset {} {rtt server stop 9090;\
    rtt stop;\
    rtt_init;}"