CFLAGS_asterix = $(CFLAGS_nrf52840)
CFLAGS_asterix += $(CFLAGS_driver_nrf52_buttons)
CFLAGS_asterix += $(CFLAGS_driver_nrf52_ls013b7dh05)
CFLAGS_asterix += $(CFLAGS_driver_nrf52_qspi_flash)
CFLAGS_asterix += $(CFLAGS_driver_nrf52_bluetooth)
CFLAGS_asterix += -Ihw/platform/asterix
CFLAGS_asterix += -DREBBLE_PLATFORM=asterix -DREBBLE_PLATFORM_TINTIN -DPBL_BW

SRCS_asterix = $(SRCS_nrf52840)
SRCS_asterix += $(SRCS_driver_nrf52_buttons)
SRCS_asterix += $(SRCS_driver_nrf52_ls013b7dh05)
SRCS_asterix += $(SRCS_driver_nrf52_qspi_flash)
SRCS_asterix += $(SRCS_driver_nrf52_bluetooth)
SRCS_asterix += hw/platform/asterix/asterix.c

LDFLAGS_asterix = $(LDFLAGS_nrf52840)
LIBS_asterix = $(LIBS_nrf52840)

#QEMUFLAGS_asterix = -machine pebble-bb2 -cpu cortex-m3
#QEMUSPITYPE_asterix = mtdblock
#QEMUPACKSIZE_asterix = 512000
#QEMUPACKOFS_asterix = 2621440
#QEMUSPINAME_asterix = aplite/3.0

HWREV_asterix = asterix

PLATFORMS += asterix
