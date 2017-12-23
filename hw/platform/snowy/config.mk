CFLAGS_snowy = $(CFLAGS_stm32f4xx)
CFLAGS_snowy += $(CFLAGS_driver_stm32_buttons)
CFLAGS_snowy += $(CFLAGS_driver_stm32_power)
CFLAGS_snowy += -Ihw/platform/snowy
CFLAGS_snowy += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=snowy -DREBBLE_PLATFORM_SNOWY

SRCS_snowy = $(SRCS_stm32f4xx)
SRCS_snowy += $(SRCS_driver_stm32_buttons)
SRCS_snowy += $(SRCS_driver_stm32_power)
SRCS_snowy += hw/platform/snowy/snowy_display.c
SRCS_snowy += hw/platform/snowy/snowy_backlight.c
SRCS_snowy += hw/platform/snowy/snowy_power.c
SRCS_snowy += hw/platform/snowy/snowy_rtc.c
SRCS_snowy += hw/platform/snowy/snowy_scanlines.c
SRCS_snowy += hw/platform/snowy/snowy_vibrate.c
SRCS_snowy += hw/platform/snowy/snowy_ambient.c
SRCS_snowy += hw/platform/snowy/snowy_ext_flash.c
SRCS_snowy += hw/platform/snowy/snowy.c
SRCS_snowy += Resources/FPGA_4.3_snowy_dumped.bin

LDFLAGS_snowy = $(LDFLAGS_stm32f4xx)
LIBS_snowy = $(LIBS_stm32f4xx)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4
QEMUSPITYPE_snowy = pflash

PLATFORMS += snowy
