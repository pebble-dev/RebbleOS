/* nrf52_bluetooth.c
 * Bluetooth softdevice routines for nRF52
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "service.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_srv_common.h"
#include "ble_db_discovery.h"

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
BLE_DB_DISCOVERY_DEF(_disc);


int sfmt(char *buf, unsigned int len, const char *ifmt, ...);
static uint8_t _name_buf[] = "Pebble Asterix LE xxxx";

static void _ppogatt_init();
static void _advertising_init();

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
    
    /* Set up PPoGATT. */
    _ppogatt_init();
    
    /* Set up advertising data. */
    _advertising_init();

    /* Should set up conn params to save battery, conn_params_init in the sample */
    
    /* And turn on advertising! */
    
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "advertising as \"%s\"", (char *)_name_buf);
    
    bluetooth_init_complete(INIT_RESP_OK);
    
    return 0;
}

/***** Advertising enable and disable *****/

static uint8_t _adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t _advdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t _srdata_buf[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static ble_gap_adv_data_t _advdata = {
    .adv_data = { .p_data = _advdata_buf, .len = sizeof(_advdata_buf) },
    .scan_rsp_data = { .p_data = _srdata_buf, .len = sizeof(_srdata_buf) },
};

static void _advertising_init() {
    hw_bluetooth_advertising_visible(0);
}

const char *hw_bluetooth_name() {
    return (const char *)_name_buf;
}

void hw_bluetooth_advertising_visible(int vis) {
    ble_advdata_t advdata;
    ble_advdata_t srdata;
    ble_advdata_manuf_data_t manufdata;
    int rv;
    
    if (_adv_handle != BLE_GAP_ADV_SET_HANDLE_NOT_SET) {
        sd_ble_gap_adv_stop(_adv_handle);
    }
    
    memset(&advdata, 0, sizeof(advdata));
    _advdata.adv_data.len = sizeof(_advdata_buf);
    advdata.name_type = vis ? BLE_ADVDATA_FULL_NAME : BLE_ADVDATA_NO_NAME;
    advdata.include_appearance = vis ? 1 : 0;
    advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    rv = ble_advdata_encode(&advdata, _advdata.adv_data.p_data, &_advdata.adv_data.len);
    assert(rv == NRF_SUCCESS && "ble_advdata_encode(advdata)");

    ble_uuid_t adv_uuids[] = {{0xFED9, BLE_UUID_TYPE_BLE}};
    memset(&srdata, 0, sizeof(srdata));
    _advdata.scan_rsp_data.len = sizeof(_srdata_buf);
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
    advparams.properties.type = /* vis ? */ BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED /*: BLE_GAP_ADV_TYPE_CONNECTABLE_NONSCANNABLE_DIRECTED */;
    advparams.p_peer_addr = NULL;
    advparams.filter_policy = /* vis ? */ BLE_GAP_ADV_FP_ANY /* : BLE_GAP_ADV_FP_FILTER_BOTH */; /* XXX: Later, whitelist things that can try to connect. */
    advparams.interval = vis ? 64 /* 40ms */ : 800 /* 500ms */;
    rv = sd_ble_gap_adv_set_configure(&_adv_handle, &_advdata, &advparams);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_set_configure");
    
    rv = sd_ble_gap_adv_start(_adv_handle, CONN_TAG);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_start");
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
 * Data characteristic (both NOTIFY phone -> pebble and WRITE NO RESPONSE pebble -> phone) UUID:
 *   10000001-328E-0FBB-C642-1AA6699BDADA
 * Unknown read characteristic (server -> client, or phone -> pebble) UUID:
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
 */

/* UUIDs are from the watch's perspective; i.e., 'srv' is for the server on
 * the watch, or what the phone would be a client for */
static uint16_t _bt_conn = BLE_CONN_HANDLE_INVALID;

static ble_uuid_t ppogatt_srv_svc_uuid;
static ble_uuid_t ppogatt_cli_svc_uuid;
static ble_uuid_t pebble_metadata_svc_uuid;

static uint16_t ppogatt_srv_svc_hnd = BLE_GATT_HANDLE_INVALID;
static ble_gatts_char_handles_t ppogatt_srv_write_hnd;
static ble_gatts_char_handles_t ppogatt_srv_read_hnd;
static ble_gatts_char_handles_t ppogatt_srv_notify_hnd;
static uint16_t ppogatt_srv_notify_cccd = 0;

static uint16_t ppogatt_cli_data_value_hnd = BLE_GATT_HANDLE_INVALID;
static uint16_t ppogatt_cli_data_cccd_hnd = BLE_GATT_HANDLE_INVALID;

#define PPOGATT_MTU 256
static uint8_t ppogatt_srv_wr_buf[PPOGATT_MTU];
static uint8_t ppogatt_srv_rd_buf[PPOGATT_MTU];

static uint16_t pebble_metadata_srv_svc_hnd = BLE_GATT_HANDLE_INVALID;
static uint8_t pebble_metadata_connectivity_buf[4] = { 0x00, 0x00, 0x00, 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_connectivity_hnd;
static uint8_t pebble_metadata_pairing_buf[1] = { 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_pairing_hnd;
static uint8_t pebble_metadata_mtu_buf[2] = { 0x00, 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_mtu_hnd;
static uint8_t pebble_metadata_parameters_buf[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_parameters_hnd;

static ble_ppogatt_callback_connected_t _ppogatt_callback_connected;
static ble_ppogatt_callback_txready_t _ppogatt_callback_txready;
static ble_ppogatt_callback_rx_t _ppogatt_callback_rx;
static ble_ppogatt_callback_disconnected_t _ppogatt_callback_disconnected;

static void _default_get_bond_data(const void *peer, size_t len) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "default bond getter, returning none");
    hw_bluetooth_bond_data_available(NULL, 0);
}

static void _default_request_bond(const void *peer, size_t len, const char *name, const void *data, size_t datalen) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "default bond request for peer %s, returning bonded", name);
    hw_bluetooth_bond_acknowledge(1);
}

static bt_callback_get_bond_data_t _bt_get_bond_data = _default_get_bond_data;
static bt_callback_request_bond_t _bt_request_bond = _default_request_bond;

static int _ppogatt_is_ready = 0;

static ble_gap_enc_key_t _bt_own_enc_key, _bt_peer_enc_key;
static ble_gap_id_key_t  _bt_own_id_key , _bt_peer_id_key;
static ble_gap_master_id_t _bt_peer_id;

static uint8_t _bt_remote_name[64];
static uint16_t _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
static int _bt_remote_name_request_queued = 0;

static int _bt_conn_is_bonded = 0;

static ble_gap_sec_keyset_t _bt_peer_keys = {
    .keys_own = {
        .p_enc_key = &_bt_own_enc_key,
        .p_id_key = &_bt_own_id_key,
        .p_pk = NULL,
        .p_sign_key = NULL,
    },
    .keys_peer = {
        .p_enc_key = &_bt_peer_enc_key,
        .p_id_key = &_bt_peer_id_key,
        .p_pk = NULL,
        .p_sign_key = NULL,
    }
};

struct nrf52_peer_data {
    ble_gap_enc_key_t enc_key_own;
    ble_gap_id_key_t  id_key_own;
};

static struct nrf52_peer_data _bt_pdata_buf;

/* The SoftDevice is not really capable of walking and chewing bubble gum at
 * the same time -- or, at least, of doing service discovery while
 * simultaneously doing a GATTC request.  So we help it out by periodically
 * retrying once it becomes no-longer-busy.  We use the service worker to
 * schedule periodic retries.
 */

static void _enqueue_remote_name_request();

static void _svc_req_remote_name(void *p) {
    if (_bt_remote_name_hnd == BLE_GATT_HANDLE_INVALID)
        return;
    
    ret_code_t rv = sd_ble_gattc_read(_bt_conn, _bt_remote_name_hnd, 0);
    if (rv == NRF_ERROR_BUSY) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote req busy, trying again");
        _bt_remote_name_request_queued = 1;
        return;
    }
    assert(rv == NRF_SUCCESS && "sd_ble_gattc_read(device name)");
}

static void _enqueue_remote_name_request() {
    if (!_bt_remote_name_request_queued)
        return;
    _bt_remote_name_request_queued = 0;
    service_submit(_svc_req_remote_name, NULL);
}

static void _hw_bluetooth_handler(const ble_evt_t *evt, void *context) {
    ret_code_t rv;
    
    _enqueue_remote_name_request();
    
    switch (evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote endpoint connected, bonded status %d", _bt_conn_is_bonded);
        _bt_conn = evt->evt.gap_evt.conn_handle;
        
        rv = ble_db_discovery_start(&_disc, _bt_conn);
        assert(rv == NRF_SUCCESS && "ble_db_discovery_start");
        
        _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
        
        ppogatt_srv_notify_cccd = 0;
        _ppogatt_is_ready = 0;
        
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote disconnected");
        _bt_conn = BLE_CONN_HANDLE_INVALID;
        _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
        ppogatt_srv_notify_cccd = 0;
        ppogatt_cli_data_value_hnd = BLE_GATT_HANDLE_INVALID;
        ppogatt_cli_data_cccd_hnd = BLE_GATT_HANDLE_INVALID;
        rv = sd_ble_gap_adv_start(_adv_handle, CONN_TAG);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_start");
        _ppogatt_callback_disconnected();
        break;
    case BLE_GAP_EVT_SEC_INFO_REQUEST: {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_SEC_INFO_REQUEST");
        memcpy(&_bt_peer_id, &evt->evt.gap_evt.params.sec_info_request.master_id, sizeof(_bt_peer_id));
        _bt_get_bond_data(&_bt_peer_id, sizeof(_bt_peer_id));
        break;
    }
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST: {
        const ble_gap_sec_params_t params = {
            .bond = 1,
            .io_caps = BLE_GAP_IO_CAPS_NONE,
            .mitm = 0,
            .kdist_own = { .enc = 1, .id = 1 },
            .kdist_peer = { .enc = 1, .id = 1 },
            .min_key_size = 7,
            .max_key_size = 16,
        };
        rv = sd_ble_gap_sec_params_reply(_bt_conn, BLE_GAP_SEC_STATUS_SUCCESS, &params, &_bt_peer_keys);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_sec_params_reply");
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "initiating pairing");
        break;
    }
    case BLE_GAP_EVT_AUTH_STATUS:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_AUTH_STATUS: bonded %d", evt->evt.gap_evt.params.auth_status.bonded);
        if (evt->evt.gap_evt.params.auth_status.bonded) {
            _bt_remote_name_request_queued = 1;
            _enqueue_remote_name_request();
        }
        break;
    case BLE_GAP_EVT_CONN_SEC_UPDATE:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_CONN_SEC_UPDATE");
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
        else if (evtwr->handle == pebble_metadata_srv_connectivity_hnd.value_handle)
            hname = "connectivity value";
        else if (evtwr->handle == pebble_metadata_srv_connectivity_hnd.cccd_handle)
            hname = "connectivity CCCD";
        else if (evtwr->handle == pebble_metadata_srv_pairing_hnd.value_handle)
            hname = "pairing value";
        else if (evtwr->handle == pebble_metadata_srv_pairing_hnd.cccd_handle)
            hname = "pairing CCCD";
        else if (evtwr->handle == pebble_metadata_srv_mtu_hnd.value_handle)
            hname = "MTU value";
        else if (evtwr->handle == pebble_metadata_srv_mtu_hnd.cccd_handle)
            hname = "MTU CCCD";
        else if (evtwr->handle == pebble_metadata_srv_parameters_hnd.value_handle)
            hname = "parameters value";
        else if (evtwr->handle == pebble_metadata_srv_parameters_hnd.cccd_handle)
            hname = "parameters CCCD";
        if (evtwr->handle != ppogatt_srv_write_hnd.value_handle)
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "GATTS write evt: handle %04x %s, op %02x, auth req %u, offset %02x, len %d", evtwr->handle, hname, evtwr->op, evtwr->auth_required, evtwr->offset, evtwr->len);
        
        if (evtwr->handle == ppogatt_srv_write_hnd.value_handle && _bt_conn_is_bonded) {
            _ppogatt_callback_rx(evtwr->data, evtwr->len);
        } else if (evtwr->handle == ppogatt_srv_notify_hnd.cccd_handle && evtwr->len <= 2) {
            memcpy(&ppogatt_srv_notify_cccd, evtwr->data, evtwr->len);
            if (ppogatt_srv_notify_cccd) {
                if (!_ppogatt_is_ready && _bt_conn_is_bonded)
                    _ppogatt_callback_connected();
                _ppogatt_is_ready = 1;
            }
        }
        break;
    }
    case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST: 
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST");
        rv = sd_ble_gatts_exchange_mtu_reply(_bt_conn, PPOGATT_MTU);
        break;
    case BLE_GATTC_EVT_READ_RSP: {
        /* Currently, the only gattc read that happens is from
         * _ble_disc_handler, where we get the remote device name.  So we
         * assume that a READ_RSP is from there.
         */
        assert(evt->evt.gattc_evt.params.read_rsp.handle == _bt_remote_name_hnd);
        int len = evt->evt.gattc_evt.params.read_rsp.len;
        if (len >= sizeof(_bt_remote_name))
            len = sizeof(_bt_remote_name) - 1;
        memcpy(_bt_remote_name, evt->evt.gattc_evt.params.read_rsp.data, len);
        _bt_remote_name[len] = 0;
        
        /* Set up the return data buffer. */
        memcpy(&_bt_pdata_buf.enc_key_own, &_bt_own_enc_key, sizeof(ble_gap_enc_key_t));
        memcpy(&_bt_pdata_buf.id_key_own,  &_bt_own_id_key,  sizeof(ble_gap_id_key_t));
        assert(!_bt_conn_is_bonded);
        _bt_request_bond(&_bt_peer_id, sizeof(_bt_peer_id), (char *)_bt_remote_name, &_bt_pdata_buf, sizeof(_bt_pdata_buf));
        break;
    }
    case BLE_GATTC_EVT_CHAR_DISC_RSP:
    case BLE_GATTC_EVT_DESC_DISC_RSP:
    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
    case BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP:
    case BLE_GATTC_EVT_WRITE_RSP:
        break;
    case BLE_GAP_EVT_PHY_UPDATE:
        /* XXX */
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_PHY_UPDATE");
        break;
    case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_CONN_PARAM_UPDATE");
        break;
    case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST");
        break;
    case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_DATA_LENGTH_UPDATE");
        break;
    case BLE_GATTC_EVT_HVX: {
        const ble_gattc_evt_hvx_t *evthvx = &evt->evt.gattc_evt.params.hvx;

        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GATTC_EVT_HVX");
        if (evthvx->handle != ppogatt_cli_data_value_hnd) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GATTC_EVT_HVX: unexpected handle %04x (hoping for %04x)?", evthvx->handle, ppogatt_cli_data_value_hnd);
            break;
        }
        
        if (_bt_conn_is_bonded)
            _ppogatt_callback_rx(evthvx->data, evthvx->len);
        break;
    }
    case BLE_GATTC_EVT_TIMEOUT:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GATTC_EVT_TIMEOUT");
        break;
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        /* XXX: may need to update this for bonded devices */
        rv = sd_ble_gatts_sys_attr_set(_bt_conn, NULL, 0, 0);
        assert(rv == NRF_SUCCESS && "sd_ble_gatts_sys_attr_set");
        break;
    case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
    case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        _ppogatt_callback_txready();
        break;
    default:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "unknown event ID %d", evt->header.evt_id);
    }
}

void hw_bluetooth_bond_data_available(const void *data, size_t datalen) {
    if (!data || (datalen != sizeof(struct nrf52_peer_data))) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "bond data not available, returning NULL");
        int rv = sd_ble_gap_sec_info_reply(_bt_conn, NULL, NULL, NULL);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_sec_info_reply");
        _bt_conn_is_bonded = 0;
        
        return;
    }
    
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "using stored bond data");
    const struct nrf52_peer_data *pdata = data;
    memcpy(&_bt_own_enc_key , &pdata->enc_key_own , sizeof(ble_gap_enc_key_t));
    memcpy(&_bt_own_id_key  , &pdata->id_key_own  , sizeof(ble_gap_id_key_t));
    int rv = sd_ble_gap_sec_info_reply(_bt_conn, &_bt_own_enc_key.enc_info, &_bt_own_id_key.id_info, NULL);
    assert(rv == NRF_SUCCESS && "sd_ble_gap_sec_info_reply");
    _bt_conn_is_bonded = 1;
}

void hw_bluetooth_bond_acknowledge(int accepted) {
    if (!accepted)
        return; /* disconnect? */
    
    assert(!_bt_conn_is_bonded);
    _bt_conn_is_bonded = 1;
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "bond accepted!");
    if (_ppogatt_is_ready)
        _ppogatt_callback_connected();
}

static void _ble_disc_handler(ble_db_discovery_evt_t *evt) {
    switch (evt->evt_type) {
    case BLE_DB_DISCOVERY_ERROR:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: error");
        break;
    case BLE_DB_DISCOVERY_SRV_NOT_FOUND:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: remote not found");
        break;
    case BLE_DB_DISCOVERY_AVAILABLE:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: available");
        break;
    case BLE_DB_DISCOVERY_COMPLETE: {
        int is_ppogatt = evt->params.discovered_db.srv_uuid.type == ppogatt_cli_svc_uuid.type;
        int is_gap = (evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE) && (evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_GAP);
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: service uuid %04x (type %d (%s)), %d chars",
            evt->params.discovered_db.srv_uuid.uuid,
            evt->params.discovered_db.srv_uuid.type,
            is_ppogatt ? "PPoGATT server" :
              is_gap ? "GAP" :
              "unknown",
            evt->params.discovered_db.char_count);
        
        for (int i = 0; i < evt->params.discovered_db.char_count; i++) {
            const ble_gatt_db_char_t *dbchar = &(evt->params.discovered_db.charateristics[i]); /* Yes, the typo is in the nRF SDK. */
            
            /* ... keep implementing GAP discovery ... */
            if (is_gap && (dbchar->characteristic.uuid.uuid == BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME)) {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: found device name characteristic w/ value handle %04x", dbchar->characteristic.handle_value);
                _bt_remote_name_hnd = dbchar->characteristic.handle_value;
            } else if (is_ppogatt && (dbchar->characteristic.uuid.uuid == 0x0001)) {
                ppogatt_cli_data_value_hnd = dbchar->characteristic.handle_value;
                ppogatt_cli_data_cccd_hnd = dbchar->cccd_handle;
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: found data characteristic w/ value handle %04x, cccd handle %04x", ppogatt_cli_data_value_hnd, ppogatt_cli_data_cccd_hnd);
                
                /* Turn on notifications.  XXX: this times out when speaking to nRF app */
                ble_gattc_write_params_t params;
                static uint16_t cccd_val = BLE_GATT_HVX_NOTIFICATION;
                
                params.flags = 0;
                params.handle = ppogatt_cli_data_cccd_hnd;
                params.len = sizeof(cccd_val);
                params.p_value = (void *)&cccd_val;
                params.offset = 0;
                params.write_op = BLE_GATT_OP_WRITE_REQ;
                
                ret_code_t rv = sd_ble_gattc_write(_bt_conn, &params);
                assert(rv == NRF_SUCCESS);
                
                if (!_ppogatt_is_ready && _bt_conn_is_bonded)
                    _ppogatt_callback_connected();
                _ppogatt_is_ready = 1;
            } else {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: other characteristic uuid %04x value handle %04x, cccd handle %04x", dbchar->characteristic.uuid.uuid, dbchar->characteristic.handle_value, dbchar->cccd_handle);
            }
        }
        break;
    }
    }
}

/* On-boot PPoGATT setup (adds GATT server on watch side). */
static void _ppogatt_init() {
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
    params.read_access = SEC_MITM /* XXX */;
    params.write_access = SEC_MITM /* XXX */;
    params.cccd_write_access = SEC_MITM /* XXX */;
    
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
    params.read_access = SEC_MITM /* XXX */;
    
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
    params.write_access = SEC_MITM /* XXX */;
    
    rv = characteristic_add(ppogatt_srv_svc_hnd, &params, &ppogatt_srv_write_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(ppogatt_srv_write)");
    
    /* Register the connectivity service. */
    ble_uuid_t md_uuid;
    
    MAKE_UUID(pebble_metadata_svc_uuid, {
        0xda, 0xda, 0x9b, 0x69, 0xa6, 0x1a, 0x42, 0xc6,
        0xbb, 0x0f, 0x8e, 0x32, 0x03, 0x00, 0x00, 0x00 });
    
    md_uuid.type = BLE_UUID_TYPE_BLE;
    md_uuid.uuid = 0xFED9;
    
    rv = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &md_uuid, &pebble_metadata_srv_svc_hnd);
    assert(rv == NRF_SUCCESS && "sd_ble_gatts_service_add(pebble_metadata_srv_svc)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0001;
    params.uuid_type = pebble_metadata_svc_uuid.type;
    params.init_len = 4;
    params.max_len = 4;
    params.is_var_len = 0;
    params.p_init_value = pebble_metadata_connectivity_buf;
    params.char_props.notify = 1;
    params.char_props.read = 1;
    params.read_access = SEC_OPEN /* XXX */;
    params.cccd_write_access = SEC_OPEN /* XXX */;
    
    rv = characteristic_add(pebble_metadata_srv_svc_hnd, &params, &pebble_metadata_srv_connectivity_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(pebble_metadata_srv_connectivity_hnd)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0002;
    params.uuid_type = pebble_metadata_svc_uuid.type;
    params.init_len = 1;
    params.max_len = 1;
    params.is_var_len = 0;
    params.p_init_value = pebble_metadata_pairing_buf;
    params.char_props.read = 1;
    params.char_props.write = 1;
    params.read_access = SEC_MITM /* XXX */;
    params.write_access = SEC_MITM /* XXX */;
    
    rv = characteristic_add(pebble_metadata_srv_svc_hnd, &params, &pebble_metadata_srv_pairing_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(pebble_metadata_srv_pairing_hnd)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0003;
    params.uuid_type = pebble_metadata_svc_uuid.type;
    params.init_len = 2;
    params.max_len = 2;
    params.is_var_len = 0;
    params.p_init_value = pebble_metadata_mtu_buf;
    params.char_props.read = 1;
    params.char_props.notify = 1;
    params.char_props.write = 1;
    params.read_access = SEC_MITM /* XXX */;
    params.write_access = SEC_MITM /* XXX */;
    params.cccd_write_access = SEC_MITM /* XXX */;
    
    rv = characteristic_add(pebble_metadata_srv_svc_hnd, &params, &pebble_metadata_srv_mtu_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(pebble_metadata_srv_mtu_hnd)");

    memset(&params, 0, sizeof(params));
    params.uuid = 0x0005;
    params.uuid_type = pebble_metadata_svc_uuid.type;
    params.init_len = 7;
    params.max_len = 7;
    params.is_var_len = 0;
    params.p_init_value = pebble_metadata_parameters_buf;
    params.char_props.read = 1;
    params.char_props.notify = 1;
    params.char_props.write = 1;
    params.read_access = SEC_OPEN /* XXX */;
    params.write_access = SEC_OPEN /* XXX */;
    params.cccd_write_access = SEC_OPEN /* XXX */;
    
    rv = characteristic_add(pebble_metadata_srv_svc_hnd, &params, &pebble_metadata_srv_parameters_hnd);
    assert(rv == NRF_SUCCESS && "characteristic_add(pebble_metadata_srv_parameters_hnd)");

    /* Listen for the server on the phone. */
    MAKE_UUID(ppogatt_cli_svc_uuid, {
        0xda, 0xda, 0x9b, 0x69, 0xa6, 0x1a, 0x42, 0xc6,
        0xbb, 0x0f, 0x8e, 0x32, 0x00, 0x00, 0x00, 0x10 });
    
    /* Also listen for the GAP UUID, so we can discover the device name. */
    ble_uuid_t gap_uuid;
    gap_uuid.type = BLE_UUID_TYPE_BLE;
    gap_uuid.uuid = BLE_UUID_GAP;

    rv = ble_db_discovery_init(_ble_disc_handler);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_init");

    rv = ble_db_discovery_evt_register(&ppogatt_cli_svc_uuid);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_evt_register(PPoGATT)");
    
    rv = ble_db_discovery_evt_register(&gap_uuid);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_evt_register(GAP)");
}


int ble_ppogatt_tx(const uint8_t *buf, size_t len) {
    ret_code_t rv;
    
    /* NOTIFY CCCD in GATTS set up?  Prefer that. */
    if (ppogatt_srv_notify_cccd & BLE_GATT_HVX_NOTIFICATION) {
        ble_gatts_hvx_params_t hvx;
        uint16_t len16 = len;
        
        memset(&hvx, 0, sizeof(hvx));
        hvx.type = BLE_GATT_HVX_NOTIFICATION;
        hvx.handle = ppogatt_srv_notify_hnd.value_handle;
        hvx.p_data = buf;
        hvx.p_len = &len16;
        
        rv = sd_ble_gatts_hvx(_bt_conn, &hvx);
        if (rv == NRF_SUCCESS)
            return 0;
        else if (rv == NRF_ERROR_INVALID_STATE) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: NOTIFY not enabled in CCCD to respond?");
            return -1;
        } else if (rv == NRF_ERROR_RESOURCES) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: SD out of resources for NOTIFY; try again later"); /* wait for BLE_GATTS_EVT_HVN_TX_COMPLETE */
            return -1;
        } else {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: unknown sd_ble_gatts_hvx error %d", rv);
            return -1;
        }
    } else if (ppogatt_cli_data_value_hnd != BLE_GATT_HANDLE_INVALID) { /* Connected to their server? */
        ble_gattc_write_params_t write;
        
        write.flags = 0;
        write.handle = ppogatt_cli_data_value_hnd;
        write.len = len;
        write.p_value = buf;
        write.offset = 0;
        write.write_op = BLE_GATT_OP_WRITE_CMD;
        
        rv = sd_ble_gattc_write(_bt_conn, &write);
        if (rv == NRF_SUCCESS)
            return 0;
        else if (rv == NRF_ERROR_RESOURCES) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: SD out of resources for WRITE; try again later"); /* wait for BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE */
            return -1;
        } else {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: unknown sd_ble_gattc_write error %d", rv);
            return -1;
        }
    } else {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT TX: no active way to tx back to phone");
        return -1;
    }
}

void bt_set_callback_get_bond_data(bt_callback_get_bond_data_t cbk) {
    _bt_get_bond_data = cbk;
}

void bt_set_callback_request_bond(bt_callback_request_bond_t cbk) {
    _bt_request_bond = cbk;
}

void ble_ppogatt_set_callback_connected(ble_ppogatt_callback_connected_t cbk) {
    _ppogatt_callback_connected = cbk;
}

void ble_ppogatt_set_callback_txready(ble_ppogatt_callback_txready_t cbk) {
    _ppogatt_callback_txready = cbk;
}

void ble_ppogatt_set_callback_rx(ble_ppogatt_callback_rx_t cbk) {
    _ppogatt_callback_rx = cbk;
}

void ble_ppogatt_set_callback_disconnected(ble_ppogatt_callback_disconnected_t cbk) {
    _ppogatt_callback_disconnected = cbk;
}
