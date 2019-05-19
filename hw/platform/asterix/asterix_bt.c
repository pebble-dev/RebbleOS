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
#include "nrf_ble_qwr.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_srv_common.h"

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
NRF_BLE_QWR_DEF(_qwr); /* XXX: do we actually need QWR?  what does this do? */

static uint8_t _adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t _advdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t _srdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static ble_gap_adv_data_t _advdata = {
    .adv_data = { .p_data = _advdata_buf, .len = sizeof(_advdata_buf) },
    .scan_rsp_data = { .p_data = _srdata_buf, .len = sizeof(_srdata_buf) },
};

int sfmt(char *buf, unsigned int len, const char *ifmt, ...);
static uint8_t _name_buf[] = "Pebble Asterix LE xxxx";

void _ppogatt_init();

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
    
    /* Set up the queued write module. */
    nrf_ble_qwr_init_t qwr_init = {0}; /* XXX: handlers? */
    rv = nrf_ble_qwr_init(&_qwr, &qwr_init);
    assert(rv == NRF_SUCCESS && "nrf_ble_qwr_init");
    
    /* Set up PPoGATT. */
    _ppogatt_init();
    
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

/***** Pairing and PPoGATT handling *****/


/* PPoGATT protocol notes:
 *
 * A Pebble can either be a GATT server or a GATT client.  By default, the
 * Pebble acts as a GATT *client*, connecting to a GATT server on the phone
 * -- this is the default for GadgetBridge, and presumably for all Android
 * devices.  However, the Pebble also exposes a GATT server with the same
 * UUID.
 *
 * If the phone acts as the server, the server's UUID is:
 *   10000000-328E-0FBB-C642-1AA6699BDADA
 * Write characteristic (client -> server, or pebble -> phone) UUID:
 *   10000001-328E-0FBB-C642-1AA6699BDADA
 * Notify characteristic (server -> client, or phone -> pebble) UUID:
 *   10000002-328E-0FBB-C642-1AA6699BDADA
 *
 * If the watch acts as the server, the server's UUID is:
 *   30000003-328e-0fbb-c642-1aa6699bdada
 * Notify (+write?) characteristic (pebble -> phone):
 *   30000004-328e-0fbb-c642-1aa6699bdada
 * Unknown read characteristic:
 *   30000005-328e-0fbb-c642-1aa6699bdada
 * Write characteristic (phone -> pebble):
 *   30000006-328e-0fbb-c642-1aa6699bdada
 *
 * The Pebble also has a metadata service:
 *   0000fed9-0000-1000-8000-00805f9b34fb
 * which has a pairing trigger characteristic (behavior unknown, read/write):
 *   00000002-328e-0fbb-c642-1aa6699bdada
 * an MTU characteristic (notify, read, write):
 *   00000003-328e-0fbb-c642-1aa6699bdada
 * a connectivity characteristic (notify, read):
 *   00000001-328e-0fbb-c642-1aa6699bdada
 * and a parameters characteristic (notify, read, write):
 *   00000005-328e-0fbb-c642-1aa6699bdada
 *
 * The PPoGATT protocol is a relatively simple shim around the Pebble
 * Protocol.  The first byte of a PPoGATT packet is a bitfield:
 *   data[7:0] = {seq[4:0], cmd[2:0]}
 *
 * cmd can have four values that we know of:
 *
 *   3'd0: Data packet with sequence `seq`.  Should be responded to with an
 *         ACK packet with the same sequence.  If a packet in sequence is
 *         missing, do not respond with any ACKs until the missing sequenced
 *         packet is retransmitted.
 *   3'd1: ACK for data packet with sequence `seq`.
 *   3'd2: Reset request. [has data unknown]
 *   3'd3: Reset ACK. [has data unknown]
 *
 * Sequences are increasing and repeating.  
 */

/* UUIDs are from the watch's perspective; i.e., 'srv' is for the server on
 * the watch, or what the phone would be a client for */
static ble_uuid_t ppogatt_srv_svc_uuid;

static uint16_t ppogatt_srv_svc_hnd;
static ble_gatts_char_handles_t ppogatt_srv_write_hnd;
static ble_gatts_char_handles_t ppogatt_srv_read_hnd;
static ble_gatts_char_handles_t ppogatt_srv_notify_hnd;

#define PPOGATT_MTU 256
static uint8_t ppogatt_srv_wr_buf[PPOGATT_MTU];
static uint8_t ppogatt_srv_rd_buf[PPOGATT_MTU];

static uint16_t _bt_conn = BLE_CONN_HANDLE_INVALID;

static void _hw_bluetooth_handler(const ble_evt_t *evt, void *context) {
    ret_code_t rv;
    
    switch (evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote endpoint connected");
        _bt_conn = evt->evt.gap_evt.conn_handle;
        rv = nrf_ble_qwr_conn_handle_assign(&_qwr, _bt_conn);
        assert(rv == NRF_SUCCESS && "nrf_ble_qwr_conn_handle_assign");
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote disconnected");
        _bt_conn = BLE_CONN_HANDLE_INVALID;
        rv = sd_ble_gap_adv_start(_adv_handle, CONN_TAG);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_start");
        break;
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        /* For now, pairing is not supported.  We'll have to do better
         * sooner or later, once we can write to flash. */
        rv = sd_ble_gap_sec_params_reply(_bt_conn, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_sec_params_reply");
        break;
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        const ble_gap_phys_t phys = {
            .rx_phys = BLE_GAP_PHY_AUTO,
            .tx_phys = BLE_GAP_PHY_AUTO
        };
        
        rv = sd_ble_gap_phy_update(evt->evt.gap_evt.conn_handle, &phys);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_phy_update");
        break;
    }
    case BLE_GATTS_EVT_WRITE: {
        const ble_gatts_evt_write_t *evtwr = &evt->evt.gatts_evt.params.write;
        const char *hname = "unknown";
        if (evtwr->handle == ppogatt_srv_write_hnd.value_handle)
            hname = "write value";
        else if (evtwr->handle == ppogatt_srv_notify_hnd.value_handle)
            hname = "notify value";
        else if (evtwr->handle == ppogatt_srv_notify_hnd.cccd_handle)
            hname = "notify CCCD";
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "GATTS write evt: handle %04x %s, op %02x, auth req %u, offset %02x, len %d", evtwr->handle, hname, evtwr->op, evtwr->auth_required, evtwr->offset, evtwr->len);
        
        /* right back atcha */
        ble_gatts_hvx_params_t hvx;
        uint16_t len = evtwr->len;
        
        memset(&hvx, 0, sizeof(hvx));
        hvx.type = BLE_GATT_HVX_NOTIFICATION;
        hvx.handle = ppogatt_srv_notify_hnd.value_handle;
        hvx.p_data = evtwr->data;
        hvx.p_len = &len;
        
        rv = sd_ble_gatts_hvx(_bt_conn, &hvx);
        if (rv == NRF_SUCCESS)
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "-> sent NOTIFY back");
        else if (rv == NRF_ERROR_INVALID_STATE)
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "NOTIFY not enabled in CCCD to respond");
        else if (rv == NRF_ERROR_RESOURCES)
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "SD out of resources for NOTIFY");
        else
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "... unknown NOTIFY error %d", rv);
        break;
    }
    default:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "unknown event ID %d", evt->header.evt_id);
    }
}

/* On-boot PPoGATT setup (adds GATT server on watch side). */
void _ppogatt_init() {
    ret_code_t rv;
    /* UUID: 30000003-328e-0fbb-c642-1aa6699bdada */
    /*   characteristics: 0004, 0005, 0006 */

    /* Add event observer for connect / disconnect / params / pairing
     * requests.  */
    NRF_SDH_BLE_OBSERVER(m_ble_observer, 3 /* priority */, _hw_bluetooth_handler, NULL);
    
    /* Create UUIDs that we might use later. */
#define MAKE_UUID(_uuid, ...) \
    do { \
        const ble_uuid128_t _uuid_base = { __VA_ARGS__ }; \
        rv = sd_ble_uuid_vs_add(&_uuid_base, &_uuid.type); \
        assert(rv == NRF_SUCCESS && "sd_ble_uuid_vs_add"); \
        _uuid.uuid = _uuid_base.uuid128[13] << 8 | _uuid_base.uuid128[12]; \
    } while(0)

    MAKE_UUID(ppogatt_srv_svc_uuid, {
        0xda, 0xda, 0x9b, 0x69, 0xa6, 0x1a, 0x42, 0xc6,
        0xbb, 0x0f, 0x8e, 0x32, 0x03, 0x00, 0x00, 0x30 });
    
    /* Register the PPoGATT server service, and add its characteristics. */
    ble_add_char_params_t params;

    rv = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ppogatt_srv_svc_uuid, &ppogatt_srv_svc_hnd);
    assert(rv == NRF_SUCCESS && "sd_ble_gatts_service_add(ppogatt_srv_svc)");
    
    memset(&params, 0, sizeof(params));
    params.uuid = 0x0004;
    params.uuid_type = ppogatt_srv_svc_uuid.type;
    params.init_len = 0;
    params.max_len = PPOGATT_MTU;
    params.is_var_len = 1;
    params.p_init_value = ppogatt_srv_rd_buf;
    params.char_props.notify = 1;
    params.char_props.write_wo_resp = 1;
    params.read_access = SEC_OPEN /* XXX */;
    params.write_access = SEC_OPEN /* XXX */;
    params.cccd_write_access = SEC_OPEN /* XXX */;
    
    rv = characteristic_add(ppogatt_srv_svc_hnd, &params, &ppogatt_srv_notify_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(ppogatt_srv_notify)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0005;
    params.uuid_type = ppogatt_srv_svc_uuid.type;
    params.init_len = 0;
    params.max_len = PPOGATT_MTU;
    params.is_var_len = 1;
    params.p_init_value = ppogatt_srv_rd_buf;
    params.char_props.read = 1;
    params.read_access = SEC_OPEN /* XXX */;
    
    rv = characteristic_add(ppogatt_srv_svc_hnd, &params, &ppogatt_srv_read_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(ppogatt_srv_write)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0006;
    params.uuid_type = ppogatt_srv_svc_uuid.type;
    params.init_len = 0;
    params.max_len = PPOGATT_MTU;
    params.is_var_len = 1;
    params.p_init_value = ppogatt_srv_wr_buf;
    params.char_props.write_wo_resp = 1;
    params.write_access = SEC_OPEN /* XXX */;
    
    rv = characteristic_add(ppogatt_srv_svc_hnd, &params, &ppogatt_srv_write_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(ppogatt_srv_write)");
}

void bt_device_request_tx(uint8_t *data, uint16_t len) {
}
