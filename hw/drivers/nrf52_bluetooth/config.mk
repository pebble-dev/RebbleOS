CFLAGS_driver_nrf52_bluetooth = -Ihw/drivers/nrf52_bluetooth

SRCS_driver_nrf52_bluetooth = hw/drivers/nrf52_bluetooth/nrf52_bluetooth.c
SRCS_driver_nrf52_bluetooth += hw/drivers/nrf52_bluetooth/nrf52_bluetooth_ppogatt.c
SRCS_driver_nrf52_bluetooth += components/softdevice/common/nrf_sdh.c
SRCS_driver_nrf52_bluetooth += components/softdevice/common/nrf_sdh_ble.c
SRCS_driver_nrf52_bluetooth += components/ble/nrf_ble_gatt/nrf_ble_gatt.c
SRCS_driver_nrf52_bluetooth += components/ble/common/ble_advdata.c
SRCS_driver_nrf52_bluetooth += components/ble/common/ble_srv_common.c
SRCS_driver_nrf52_bluetooth += components/ble/nrf_ble_qwr/nrf_ble_qwr.c
SRCS_driver_nrf52_bluetooth += components/ble/ble_db_discovery/ble_db_discovery.c
