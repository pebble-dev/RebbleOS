CFLAGS_snowy = $(CFLAGS_stm32f4xx)
CFLAGS_snowy += -Ihw/platform/snowy
CFLAGS_snowy += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=snowy

SRCS_snowy = $(SRCS_stm32f4xx)
SRCS_snowy += hw/platform/snowy/snowy_display.c
SRCS_snowy += hw/platform/snowy/snowy_backlight.c
SRCS_snowy += hw/platform/snowy/snowy_buttons.c
SRCS_snowy += hw/platform/snowy/snowy_power.c
SRCS_snowy += hw/platform/snowy/snowy_rtc.c
SRCS_snowy += hw/platform/snowy/snowy_scanlines.c
SRCS_snowy += hw/platform/snowy/snowy_vibrate.c
SRCS_snowy += hw/platform/snowy/snowy_ambient.c
SRCS_snowy += hw/platform/snowy/snowy.c
SRCS_snowy += Resources/FPGA_4.3_snowy_dumped.bin

LDFLAGS_snowy = $(LDFLAGS_stm32f4xx)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4

PLATFORMS += snowy
