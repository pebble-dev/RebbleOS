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
#include "nrf_ble_gatt.h"
#include "ble_advdata.h"
#include "ble_gap.h"

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

NRF_BLE_GATT_DEF(_gatt);

static uint8_t _adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t _advdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t _srdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static ble_gap_adv_data_t _advdata = {
    .adv_data = { .p_data = _advdata_buf, .len = sizeof(_advdata_buf) },
    .scan_rsp_data = { .p_data = _srdata_buf, .len = sizeof(_srdata_buf) },
};

int sfmt(char *buf, unsigned int len, const char *ifmt, ...);
static uint8_t _name_buf[] = "Pebble Asterix LE xxxx";

static void _hw_bluetooth_handler(const ble_evt_t *evt, void *context) {
    printf("*** BLUETOOTH HANDLER ***\r\n");
}

uint8_t hw_bluetooth_init() {
    ret_code_t rv;
    
    /* XXX: need to ensure that we have 1536 bytes of stack available on the main stack */ 
    
    /* Turn on the softdevice. */
    rv = nrf_sdh_enable_request();
    assert(rv == NRF_SUCCESS && "nrf_sdh_enable_request");
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "softdevice is enabled");

    
    extern uint8_t __data_start__;
    uint32_t rebbleos_ram_start = (uint32_t)&__data_start__;
    uint32_t ram_start = 0;
    rv = nrf_sdh_ble_default_cfg_set(CONN_TAG, &ram_start);
    assert(rv == NRF_SUCCESS && "nrf_sdh_ble_default_cfg_set");
    if (ram_start > rebbleos_ram_start) {
        panic("BLE subsystem requires too much RAM");
    }
    
    rv = nrf_sdh_ble_enable(&rebbleos_ram_start);
    assert(rv == NRF_SUCCESS && "nrf_sdh_ble_enable");
    
    NRF_SDH_BLE_OBSERVER(m_ble_observer, 3 /* priority */, _hw_bluetooth_handler, NULL);
    
    /* Set up device name. 
     * XXX: Open link can change device mode characteristic.  Should this be (0,0)? */
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    ble_gap_addr_t addr;
    (void) sd_ble_gap_addr_get(&addr);
    sfmt((char *)_name_buf + strlen((char *)_name_buf) - 4, 5, "%02x%02x", addr.addr[1], addr.addr[0]);
    
    rv = sd_ble_gap_device_name_set(&sec_mode, _name_buf, strlen((char *)_name_buf));
    assert(rv == NRF_SUCCESS && "sd_ble_gap_device_name_set");
    
    (void) sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_WATCH);
    
    /* Set up preferred peripheral connection parameters. */
    ble_gap_conn_params_t gap_conn_params;
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS);
    gap_conn_params.conn_sup_timeout = MSEC_TO_UNITS(4000, UNIT_10_MS);
    gap_conn_params.slave_latency = 0;
    rv = sd_ble_gap_ppcp_set(&gap_conn_params);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_ppcp_set");
    
    /* Set up GATT parameter negotiation. */
    rv = nrf_ble_gatt_init(&_gatt, NULL);
    assert(rv == NRF_SUCCESS && "nrf_ble_gatt_init");
    
    /* services_init */
    
    /* Set up advertising data. */
    ble_advdata_t advdata;
    ble_advdata_t srdata;
    ble_advdata_manuf_data_t manufdata;
    
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = 1;
    advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    rv = ble_advdata_encode(&advdata, _advdata.adv_data.p_data, &_advdata.adv_data.len);
    assert(rv == NRF_SUCCESS && "ble_advdata_encode(advdata)");

    ble_uuid_t adv_uuids[] = {{0xFED9, BLE_UUID_TYPE_BLE}};
    memset(&srdata, 0, sizeof(srdata));
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids = adv_uuids;
    manufdata.company_identifier = 0x0154 /* Pebble Technology */;
    manufdata.data.size = 0;
    srdata.p_manuf_specific_data = &manufdata;
    rv = ble_advdata_encode(&srdata, _advdata.scan_rsp_data.p_data, &_advdata.scan_rsp_data.len);
    assert(rv == NRF_SUCCESS && "ble_advdata_encode(srdata)");
    
    /* Set up advertising. */
    ble_gap_adv_params_t advparams;
    
    memset(&advparams, 0, sizeof(advparams));
    advparams.primary_phy = BLE_GAP_PHY_1MBPS;
    advparams.duration = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
    advparams.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    advparams.p_peer_addr = NULL;
    advparams.filter_policy = BLE_GAP_ADV_FP_ANY;
    advparams.interval = 64; /* 40ms */
    rv = sd_ble_gap_adv_set_configure(&_adv_handle, &_advdata, &advparams);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_set_configure");
    
    /* Should set up conn params to save battery, conn_params_init in the sample */
    
    /* And turn on advertising! */
    rv = sd_ble_gap_adv_start(_adv_handle, CONN_TAG);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_start");
    
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "advertising as \"%s\"", (char *)_name_buf);
    
    return 0;
}

void bt_device_request_tx(uint8_t *data, uint16_t len) {
}
