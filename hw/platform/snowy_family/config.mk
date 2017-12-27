CFLAGS_snowy_family = $(CFLAGS_stm32f4xx)
CFLAGS_snowy_family += $(CFLAGS_driver_stm32_buttons)
CFLAGS_snowy_family += $(CFLAGS_driver_stm32_power)
CFLAGS_snowy_family += -Ihw/platform/snowy_family

SRCS_snowy_family = $(SRCS_stm32f4xx)
SRCS_snowy_family += $(SRCS_driver_stm32_buttons)
SRCS_snowy_family += $(SRCS_driver_stm32_power)
SRCS_snowy_family += hw/platform/snowy_family/snowy_display.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_backlight.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_power.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_rtc.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_scanlines.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_vibrate.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_ambient.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_ext_flash.c
SRCS_snowy_family += hw/platform/snowy_family/snowy_common.c

LDFLAGS_snowy_family = $(LDFLAGS_stm32f4xx)
LIBS_snowy_family = $(LIBS_stm32f4xx)

