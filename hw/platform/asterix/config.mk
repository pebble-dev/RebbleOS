# Base configuration for Asterix-like boards.

$(eval $(call platform_include,asterix_common,rebbleos))
$(eval $(call platform_include,asterix_common,nrf52840))
$(eval $(call platform_include,asterix_common,nrf52840_os))
$(eval $(call platform_include,asterix_common,driver_nrf52_buttons))
$(eval $(call platform_include,asterix_common,driver_nrf52_ls013b7dh05))
$(eval $(call platform_include,asterix_common,driver_nrf52_qspi_flash))
$(eval $(call platform_include,asterix_common,driver_nrf52_bluetooth))

CFLAGS_asterix_common += -Ihw/platform/asterix
CFLAGS_asterix_common += -DREBBLE_PLATFORM=asterix -DREBBLE_PLATFORM_TINTIN -DPBL_BW

SRCS_asterix_common += hw/platform/asterix/asterix.c

$(eval $(call platform_include,asterix_boot_common,boot))
$(eval $(call platform_include,asterix_boot_common,nrf52840))
$(eval $(call platform_include,asterix_boot_common,driver_nrf52_buttons))
$(eval $(call platform_include,asterix_boot_common,driver_nrf52_ls013b7dh05))
$(eval $(call platform_include,asterix_boot_common,driver_nrf52_qspi_flash))

CFLAGS_asterix_boot_common += -Ihw/platform/asterix
CFLAGS_asterix_boot_common += -DREBBLE_PLATFORM=asterix -DREBBLE_PLATFORM_TINTIN -DPBL_BW

SRCS_asterix_boot_common += hw/platform/asterix/asterix.c

$(eval $(call platform_define,asterix))
$(eval $(call platform_include,asterix,asterix_common))

HWREV_asterix = asterix
CFLAGS_asterix += -DASTERIX_BOARD_ASTERIX

$(eval $(call platform_define,asterix_vla_dvb1))
$(eval $(call platform_include,asterix_vla_dvb1,asterix_common))

HWREV_asterix_vla_dvb1 = asterix_vla_dvb1
CFLAGS_asterix_vla_dvb1 += -DASTERIX_BOARD_VLA_DVB1

$(eval $(call platform_define,asterix_vla_dvb1_boot))
$(eval $(call platform_include,asterix_vla_dvb1_boot,asterix_boot_common))

HWREV_asterix_vla_dvb1_boot = asterix_vla_dvb1
CFLAGS_asterix_vla_dvb1_boot += -DASTERIX_BOARD_VLA_DVB1

