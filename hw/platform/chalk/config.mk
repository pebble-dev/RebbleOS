CFLAGS_chalk = $(CFLAGS_stm32f4xx)
CFLAGS_chalk += $(CFLAGS_driver_stm32_buttons)
CFLAGS_chalk += $(CFLAGS_driver_stm32_power)
CFLAGS_chalk += -Ihw/platform/chalk
CFLAGS_chalk += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=chalk -DREBBLE_PLATFORM_chalk

SRCS_chalk = $(SRCS_stm32f4xx)
SRCS_chalk += $(SRCS_driver_stm32_buttons)
SRCS_chalk += $(SRCS_driver_stm32_power)
SRCS_chalk += hw/platform/chalk/chalk_display.c
SRCS_chalk += hw/platform/chalk/chalk_backlight.c
SRCS_chalk += hw/platform/snowy-family/snowy_power.c
SRCS_chalk += hw/platform/snowy-family/snowy_rtc.c
SRCS_chalk += hw/platform/chalk/chalk_scanlines.c
SRCS_chalk += hw/platform/snowy-family/snowy_vibrate.c
SRCS_chalk += hw/platform/snowy-family/snowy_ambient.c
SRCS_chalk += hw/platform/snowy-family/snowy_ext_flash.c
SRCS_chalk += hw/platform/chalk/chalk.c
SRCS_chalk += Resources/FPGA_4.3_snowy_dumped.bin

LDFLAGS_chalk = $(LDFLAGS_stm32f4xx)
LIBS_chalk = $(LIBS_stm32f4xx)

QEMUFLAGS_chalk = -machine pebble-s4-bb -cpu cortex-m4
QEMUSPITYPE_chalk = pflash

PLATFORMS += chalk