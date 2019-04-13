/* asterix_bt.c
 * Bluetooth softdevice routines for aWatch2
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

/* external callbacks:
 *   bluetooth_tx_complete_from_isr
 *   bluetooth_init_complete(INIT_RESP_OK / 
 *   bluetooth_device_connected
 *   bluetooth_device_disconnected
 *   bluetooth_data_rx(uint8_t *data, uint16_t len)
 */

/* S140 softdevice requires:
 *   1536 bytes of stack
 *   0x1628 bytes of RAM, plus sd_ble_enable-required memory
 */

#define CONN_TAG 1

static void _hw_bluetooth_handler(const ble_evt_t *evt, void *context) {
}

uint8_t hw_bluetooth_init() {
    ret_code_t rv;
    
    /* XXX: need to ensure that we have 1536 bytes of stack available on the main stack */ 
    
    /* Turn on the softdevice. */
    rv = nrf_sdh_enable_request();
    assert(rv == NRF_SUCCESS && "nrf_sdh_enable_request");
    
    extern uint8_t __data_start__;
    uint32_t rebbleos_ram_start = (uint32_t)&__data_start__;
    uint32_t ram_start = 0;
    rv = nrf_sdh_ble_default_cfg_set(CONN_TAG, &ram_start);
    assert(rv == NRF_SUCCESS && "nrf_sdh_ble_default_cfg_set");
    if (ram_start > rebbleos_ram_start) {
        panic("BLE subsystem requires too much RAM");
    }
    DRV_LOG("bt", APP_LOG_LEVEL_DEBUG, "Bluetooth stack has %d bytes of margin", rebbleos_ram_start - ram_start);
    
    rv = nrf_sdh_ble_enable(&ram_start);
    assert(rv == NRF_SUCCESS && "nrf_sdh_ble_enable");
    
    NRF_SDH_BLE_OBSERVER(m_ble_observer, 3 /* priority */, _hw_bluetooth_handler, NULL);
    
    /* gap_params_init */
    
    /* gatt_init */
    
    /* services_init */
    
    /* advertising_init */
    
    /* conn_params_init */
    
    /* advertising_start */
    
    return 0;
}

void bt_device_request_tx(uint8_t *data, uint16_t len) {
}
