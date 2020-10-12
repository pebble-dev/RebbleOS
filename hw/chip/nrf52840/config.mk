ifeq ($(NRF52_SDK_PATH),)
$(error NRF52_SDK_PATH is empty, but must not be for nrf52840 build)
endif

vpath % $(NRF52_SDK_PATH)

nrf52_sdk_inc_folders = \
	components/nfc/ndef/generic/message \
	components/nfc/t2t_lib \
	components/nfc/t4t_parser/hl_detection_procedure \
	components/ble/ble_services/ble_ancs_c \
	components/ble/ble_services/ble_ias_c \
	components/libraries/pwm \
	components/libraries/usbd/class/cdc/acm \
	components/libraries/usbd/class/hid/generic \
	components/libraries/usbd/class/msc \
	components/libraries/usbd/class/hid \
	modules/nrfx/hal \
	components/nfc/ndef/conn_hand_parser/le_oob_rec_parser \
	components/libraries/log \
	components/ble/ble_services/ble_gls \
	components/libraries/fstorage \
	components/nfc/ndef/text \
	components/libraries/mutex \
	components/libraries/gpiote \
	components/libraries/bootloader/ble_dfu \
	components/nfc/ndef/connection_handover/common \
	components/boards \
	components/nfc/ndef/generic/record \
	components/nfc/t4t_parser/cc_file \
	components/ble/ble_advertising \
	external/utf_converter \
	components/ble/ble_services/ble_bas_c \
	modules/nrfx/drivers/include \
	components/libraries/experimental_task_manager \
	components/ble/ble_services/ble_hrs_c \
	components/softdevice/s140/headers/nrf52 \
	components/nfc/ndef/connection_handover/le_oob_rec \
	components/libraries/queue \
	components/libraries/pwr_mgmt \
	components/ble/ble_dtm \
	components/toolchain/cmsis/include \
	components/ble/ble_services/ble_rscs_c \
	components/ble/common \
	components/ble/ble_services/ble_lls \
	components/nfc/ndef/connection_handover/ac_rec \
	components/ble/ble_services/ble_bas \
	components/libraries/mpu \
	components/libraries/experimental_section_vars \
	components/ble/ble_services/ble_ans_c \
	components/libraries/slip \
	components/libraries/delay \
	components/libraries/mem_manager \
	components/libraries/csense_drv \
	components/libraries/memobj \
	components/ble/ble_services/ble_nus_c \
	components/softdevice/common \
	components/ble/ble_services/ble_ias \
	components/libraries/usbd/class/hid/mouse \
	components/libraries/low_power_pwm \
	components/nfc/ndef/conn_hand_parser/ble_oob_advdata_parser \
	components/ble/ble_services/ble_dfu \
	external/fprintf \
	components/libraries/svc \
	components/libraries/atomic \
	components \
	components/libraries/scheduler \
	components/libraries/cli \
	components/ble/ble_services/ble_lbs \
	components/ble/ble_services/ble_hts \
	components/libraries/crc16 \
	components/nfc/t4t_parser/apdu \
	components/libraries/util \
	components/libraries/usbd/class/cdc \
	components/libraries/csense \
	components/libraries/balloc \
	components/libraries/ecc \
	components/libraries/hardfault \
	components/ble/ble_services/ble_cscs \
	components/libraries/hci \
	components/libraries/timer \
	components/softdevice/s140/headers \
	integration/nrfx \
	components/nfc/t4t_parser/tlv \
	components/libraries/sortlist \
	components/libraries/spi_mngr \
	components/libraries/led_softblink \
	components/nfc/ndef/conn_hand_parser \
	components/libraries/sdcard \
	components/nfc/ndef/parser/record \
	modules/nrfx/mdk \
	components/ble/ble_services/ble_cts_c \
	components/ble/ble_services/ble_nus \
	components/libraries/twi_mngr \
	components/ble/ble_services/ble_hids \
	components/libraries/strerror \
	components/libraries/crc32 \
	components/nfc/ndef/connection_handover/ble_oob_advdata \
	components/nfc/t2t_parser \
	components/nfc/ndef/connection_handover/ble_pair_msg \
	components/libraries/usbd/class/audio \
	components/nfc/t4t_lib/hal_t4t \
	components/nfc/t4t_lib \
	components/ble/peer_manager \
	components/drivers_nrf/usbd \
	components/libraries/ringbuf \
	components/ble/ble_services/ble_tps \
	components/nfc/ndef/parser/message \
	components/ble/ble_services/ble_dis \
	components/nfc/ndef/uri \
	components/ble/nrf_ble_gatt \
	components/ble/nrf_ble_qwr \
	components/libraries/gfx \
	components/libraries/button \
	modules/nrfx \
	components/libraries/twi_sensor \
	integration/nrfx/legacy \
	components/libraries/usbd/class/hid/kbd \
	components/nfc/ndef/connection_handover/ep_oob_rec \
	external/segger_rtt \
	components/libraries/atomic_fifo \
	components/ble/ble_services/ble_lbs_c \
	components/nfc/ndef/connection_handover/ble_pair_lib \
	components/libraries/crypto \
	components/ble/ble_racp \
	components/libraries/fds \
	components/nfc/ndef/launchapp \
	components/libraries/atomic_flags \
	components/ble/ble_services/ble_hrs \
	components/ble/ble_services/ble_rscs \
	components/nfc/ndef/connection_handover/hs_rec \
	components/nfc/t2t_lib/hal_t2t \
	components/libraries/usbd \
	components/nfc/ndef/conn_hand_parser/ac_rec_parser \
	components/libraries/stack_guard \
	components/libraries/log/src \
	components/libraries/cli/uart \
	components/ble/ble_db_discovery \
	components/ble/nrf_ble_gq \
	external/freertos/portable/CMSIS/nrf52

CFLAGS_nrf52840 += -Ihw/chip/nrf52840
CFLAGS_nrf52840 += $(addprefix -I$(NRF52_SDK_PATH)/,$(nrf52_sdk_inc_folders))
CFLAGS_nrf52840 += \
	-DCONFIG_GPIO_AS_PINRESET \
	-DFLOAT_ABI_HARD \
	-DNRF52840_XXAA \
	-DNRF_SD_BLE_API_VERSION=6 \
	-DS140 \
	-DSOFTDEVICE_PRESENT \
	-DSWI_DISABLE0 \
	-D__STARTUP_CLEAR_BSS \
	-D__START=main \
	-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

SRCS_nrf52840 += modules/nrfx/mdk/gcc_startup_nrf52840.S
SRCS_nrf52840 += modules/nrfx/mdk/system_nrf52840.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_uart.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_qspi.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_spim.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_twi.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_clock.c
SRCS_nrf52840 += modules/nrfx/drivers/src/nrfx_wdt.c
SRCS_nrf52840 += hw/chip/nrf52840/fault_handlers.c
SRCS_nrf52840 += hw/chip/nrf52840/debug.c
SRCS_nrf52840 += components/libraries/util/app_util_platform.c # needed by softdevice

LDFLAGS_nrf52840_boot += -Wl,-Thw/chip/nrf52840/nrf52840_boot.lds -L$(NRF52_SDK_PATH)/modules/nrfx/mdk

SRCS_nrf52840_os += components/libraries/experimental_section_vars/nrf_section_iter.c # needed by softdevice
SRCS_nrf52840_os += external/freertos/portable/GCC/nrf52/port.c
SRCS_nrf52840_os += external/freertos/portable/CMSIS/nrf52/port_cmsis.c
SRCS_nrf52840_os += external/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c
SRCS_nrf52840_os += integration/nrfx/legacy/nrf_drv_clock.c
LDFLAGS_nrf52840_os += -Wl,-Thw/chip/nrf52840/nrf52840.lds -L$(NRF52_SDK_PATH)/modules/nrfx/mdk
