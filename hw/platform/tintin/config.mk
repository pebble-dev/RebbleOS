CFLAGS_tintin = $(CFLAGS_stm32f2xx)
CFLAGS_tintin += $(CFLAGS_driver_stm32_buttons)
CFLAGS_tintin += -Ihw/platform/tintin
CFLAGS_tintin += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=tintin -DREBBLE_PLATFORM_TINTIN -DPBL_BW

SRCS_tintin = $(SRCS_stm32f2xx)
SRCS_tintin += $(SRCS_driver_stm32_buttons)
SRCS_tintin += hw/platform/tintin/tintin.c
SRCS_tintin += hw/platform/tintin/tintin_asm.s

LDFLAGS_tintin = $(LDFLAGS_stm32f2xx)
LIBS_tintin = $(LIBS_stm32f2xx)

QEMUFLAGS_tintin = -machine pebble-bb2 -cpu cortex-m3
QEMUSPITYPE_tintin = mtdblock

PLATFORMS += tintin
