# Base configuration for Asterix-like boards.

CFLAGS_asterix_common = $(CFLAGS_nrf52840)
CFLAGS_asterix_common += $(CFLAGS_driver_nrf52_buttons)
CFLAGS_asterix_common += $(CFLAGS_driver_nrf52_ls013b7dh05)
CFLAGS_asterix_common += $(CFLAGS_driver_nrf52_qspi_flash)
CFLAGS_asterix_common += $(CFLAGS_driver_nrf52_bluetooth)
CFLAGS_asterix_common += -Ihw/platform/asterix
CFLAGS_asterix_common += -DREBBLE_PLATFORM=asterix -DREBBLE_PLATFORM_TINTIN -DPBL_BW

SRCS_asterix_common = $(SRCS_nrf52840)
SRCS_asterix_common += $(SRCS_driver_nrf52_buttons)
SRCS_asterix_common += $(SRCS_driver_nrf52_ls013b7dh05)
SRCS_asterix_common += $(SRCS_driver_nrf52_qspi_flash)
SRCS_asterix_common += $(SRCS_driver_nrf52_bluetooth)
SRCS_asterix_common += hw/platform/asterix/asterix.c

LDFLAGS_asterix_common = $(LDFLAGS_nrf52840)
LIBS_asterix_common = $(LIBS_nrf52840)

#QEMUFLAGS_asterix_common = -machine pebble-bb2 -cpu cortex-m3
#QEMUSPITYPE_asterix_common = mtdblock
#QEMUPACKSIZE_asterix_common = 512000
#QEMUPACKOFS_asterix_common = 2621440
#QEMUSPINAME_asterix_common = aplite/3.0

CFLAGS_asterix = $(CFLAGS_asterix_common)
SRCS_asterix = $(SRCS_asterix_common)
LDFLAGS_asterix = $(LDFLAGS_asterix_common)
LIBS_asterix = $(LIBS_asterix_common)
HWREV_asterix = asterix

CFLAGS_asterix += -DASTERIX_BOARD_ASTERIX

PLATFORMS += asterix

CFLAGS_asterix_vla_dvb1 = $(CFLAGS_asterix_common)
CFLAGS_asterix_vla_dvb1 += $(CFLAGS_driver_nrf52_ls013b7dh05)
SRCS_asterix_vla_dvb1 = $(SRCS_asterix_common)
SRCS_asteri_vla_dvb1 += $(SRCS_driver_nrf52_ls013b7dh05)
LDFLAGS_asterix_vla_dvb1 = $(LDFLAGS_asterix_common)
LIBS_asterix_vla_dvb1 = $(LIBS_asterix_common)
HWREV_asterix_vla_dvb1 = asterix_vla_dvb1

CFLAGS_asterix_vla_dvb1 += -DASTERIX_BOARD_VLA_DVB1

PLATFORMS += asterix_vla_dvb1

CFLAGS_asterix_vla_dvb2 = $(CFLAGS_asterix_common)
CFLAGS_asterix_vla_dvb2 += $(CFLAGS_driver_nrf52_ls013b7dh05)
SRCS_asterix_vla_dvb2 = $(SRCS_asterix_common)
SRCS_asterix_vla_dvb2 += $(SRCS_driver_nrf52_ls013b7dh05)
LDFLAGS_asterix_vla_dvb2 = $(LDFLAGS_asterix_common)
LIBS_asterix_vla_dvb2 = $(LIBS_asterix_common)
HWREV_asterix_vla_dvb2 = asterix_vla_dvb2

CFLAGS_asterix_vla_dvb2 += -DASTERIX_BOARD_VLA_DVB2

PLATFORMS += asterix_vla_dvb2
