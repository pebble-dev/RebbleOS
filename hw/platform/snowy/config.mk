CFLAGS_snowy = $(CFLAGS_snowy_family)
CFLAGS_snowy += $(CFLAGS_bt)
CFLAGS_snowy += $(CFLAGS_driver_stm32_bluetooth_cc256x)
CFLAGS_snowy += -Ihw/platform/snowy
CFLAGS_snowy += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=snowy -DREBBLE_PLATFORM_SNOWY

SRCS_snowy = $(SRCS_snowy_family)
SRCS_snowy += $(SRCS_bt)
SRCS_snowy += $(SRCS_driver_stm32_bluetooth_cc256x)
SRCS_snowy += hw/platform/snowy/snowy.c
SRCS_snowy += hw/platform/snowy/snowy_bluetooth.c
SRCS_snowy += Resources/snowy_fpga.bin

LDFLAGS_snowy = $(LDFLAGS_snowy_family)
LIBS_snowy = $(LIBS_snowy_family)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4
QEMUSPITYPE_snowy = pflash
QEMUPACKSIZE_snowy = 512000
QEMUPACKOFS_snowy = 3670016
QEMUSPINAME_snowy = basalt/3.0

HWREV_snowy = snowy_dvt

TESTABLE_snowy = true

PLATFORMS += snowy
