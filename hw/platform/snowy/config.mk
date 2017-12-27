CFLAGS_snowy = $(CFLAGS_snowy_family)
CFLAGS_snowy += -Ihw/platform/snowy
CFLAGS_snowy += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=snowy -DREBBLE_PLATFORM_SNOWY

SRCS_snowy = $(SRCS_snowy_family)
SRCS_snowy += hw/platform/snowy/snowy.c

LDFLAGS_snowy = $(LDFLAGS_snowy_family)
LIBS_snowy = $(LIBS_snowy_family)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4
QEMUSPITYPE_snowy = pflash

PLATFORMS += snowy
