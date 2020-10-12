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
#include "nrf_sdm.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_srv_common.h"
#include "ble_db_discovery.h"
#include "nrf52_bluetooth_internal.h"

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
NRF_BLE_GQ_DEF(_gatt_queue, NRF_SDH_BLE_PERIPHERAL_LINK_COUNT, NRF_BLE_GQ_QUEUE_SIZE);

int sfmt(char *buf, unsigned int len, const char *ifmt, ...);
static uint8_t _name_buf[] = "Pebble Asterix LE xxxx";

static void _advertising_init();
static void _pairing_init();

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
    
    /* Set up pairing state machine. */
    _pairing_init();
    
    /* Set up PPoGATT. */
    nrf52_ppogatt_init();
    
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

static uint8_t _pebble_magic_data[] = {
    0x00, 'C', '5', '5', '6', '4', '1', '6', 'A', '1', 'S', '0', 'B',
    0x0e, 0x19, 0x04, 0x03, 0x00, 0x00
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
    manufdata.company_identifier = 0x0154 /* Pebble Technology */;
    manufdata.data.size = sizeof(_pebble_magic_data);
    manufdata.data.p_data = _pebble_magic_data;
    rv = ble_advdata_encode(&advdata, _advdata.adv_data.p_data, &_advdata.adv_data.len);
    assert(rv == NRF_SUCCESS && "ble_advdata_encode(advdata)");

    ble_uuid_t adv_uuids[] = {{0xFED9, BLE_UUID_TYPE_BLE}};
    memset(&srdata, 0, sizeof(srdata));
    _advdata.scan_rsp_data.len = sizeof(_srdata_buf);
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids = adv_uuids;
    manufdata.company_identifier = 0x0154 /* Pebble Technology */;
    manufdata.data.size = sizeof(_pebble_magic_data);
    manufdata.data.p_data = _pebble_magic_data;
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

/***** Pairing and bonding *****/

static uint16_t _bt_conn = BLE_CONN_HANDLE_INVALID;

static bt_callback_get_bond_data_t _bt_get_bond_data;
static bt_callback_request_bond_t _bt_request_bond;

static ble_gap_enc_key_t _bt_own_enc_key, _bt_peer_enc_key;
static ble_gap_id_key_t  _bt_own_id_key , _bt_peer_id_key;
static ble_gap_master_id_t _bt_peer_id;

static uint8_t _bt_remote_name[64];
static uint16_t _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
static int _bt_remote_name_request_queued = 0;
static int _bt_remote_name_request_desired = 0;

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

static void _gatt_error_handler(uint32_t nrf_error, void *p_ctx, uint16_t conn_handle) {
    DRV_LOG("bt", APP_LOG_LEVEL_ERROR, "gatt error handler: %d\n", nrf_error);
}

static void _svc_req_remote_name(void *p) {
    if (_bt_remote_name_hnd == BLE_GATT_HANDLE_INVALID)
        return;
    
    nrf_ble_gq_req_t gq_req = {
        .type = NRF_BLE_GQ_REQ_GATTC_READ,
        .error_handler = {
            .cb = _gatt_error_handler
        },
        .params = {
            .gattc_read = {
                .handle = _bt_remote_name_hnd,
                .offset = 0
            }
        }
    };
    
    ret_code_t rv = nrf_ble_gq_item_add(&_gatt_queue, &gq_req, _bt_conn);
    if (rv == NRF_ERROR_NO_MEM) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote req busy, trying again");
        _bt_remote_name_request_queued = 1;
        return;
    }
    assert(rv == NRF_SUCCESS && "sd_ble_gattc_read(device name)");
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "ok, enqueued one");
}

static void _enqueue_remote_name_request() {
    if (!_bt_remote_name_request_queued)
        return;
    _bt_remote_name_request_queued = 0;
    service_submit(_svc_req_remote_name, NULL, 0);
}

const ble_gap_sec_params_t _gap_sec_params = {
    .bond = 1,
    .io_caps = BLE_GAP_IO_CAPS_NONE,
    .mitm = 0,
    .kdist_own = { .enc = 1, .id = 1 },
    .kdist_peer = { .enc = 1, .id = 1 },
    .min_key_size = 7,
    .max_key_size = 16,
};

static void _pairing_handler(const ble_evt_t *evt, void *context) {
    ret_code_t rv;
    
    _enqueue_remote_name_request();
    
    switch (evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote endpoint connected, bonded status %d", _bt_conn_is_bonded);
        _bt_conn = evt->evt.gap_evt.conn_handle;
        _bt_conn_is_bonded = 0;
        _bt_remote_name_request_desired = 0;

        /* Begin the pairing process. */
        sd_ble_gap_authenticate(_bt_conn, &_gap_sec_params);
        
        _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
        
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "remote disconnected");
        _bt_conn = BLE_CONN_HANDLE_INVALID;
        _bt_remote_name_hnd = BLE_GATT_HANDLE_INVALID;
        rv = sd_ble_gap_adv_start(_adv_handle, CONN_TAG);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_adv_start");
        break;
    case BLE_GAP_EVT_SEC_INFO_REQUEST:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_SEC_INFO_REQUEST");
        memcpy(&_bt_peer_id, &evt->evt.gap_evt.params.sec_info_request.master_id, sizeof(_bt_peer_id));
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "  asking about ediv %04x, rand %02x %02x", _bt_peer_id.ediv, _bt_peer_id.rand[0], _bt_peer_id.rand[1]);
        _bt_get_bond_data(&_bt_peer_id, sizeof(_bt_peer_id));
        break;
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_SEC_PARAMS_REQUEST");
        rv = sd_ble_gap_sec_params_reply(_bt_conn, BLE_GAP_SEC_STATUS_SUCCESS, &_gap_sec_params, &_bt_peer_keys);
        assert(rv == NRF_SUCCESS && "sd_ble_gap_sec_params_reply");
        break;
    case BLE_GAP_EVT_AUTH_STATUS:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_AUTH_STATUS: bonded %d", evt->evt.gap_evt.params.auth_status.bonded);
        if (evt->evt.gap_evt.params.auth_status.bonded) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "  own  ediv %04x, rand %02x %02x", _bt_own_enc_key.master_id.ediv, _bt_own_enc_key.master_id.rand[0], _bt_own_enc_key.master_id.rand[1]);
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "  peer ediv %04x, rand %02x %02x", _bt_peer_enc_key.master_id.ediv, _bt_peer_enc_key.master_id.rand[0], _bt_peer_enc_key.master_id.rand[1]);
            memcpy(&_bt_peer_id, &_bt_own_enc_key.master_id, sizeof(_bt_peer_id));

            _bt_remote_name_request_desired = 1;
        }
        break;
    case BLE_GAP_EVT_CONN_SEC_UPDATE:
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GAP_EVT_CONN_SEC_UPDATE");
        
        /* If we do this before the secure link comes up while we're
         * bonded, we croak for some reason.  For some reason it's okay
         * if the Central does it, just not if we do it...
         */
        _bt_remote_name_request_desired = 0;
        rv = ble_db_discovery_start(&_disc, _bt_conn);
        assert(rv == NRF_SUCCESS && "ble_db_discovery_start");
        
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
    case BLE_GATTC_EVT_CHAR_DISC_RSP:
    case BLE_GATTC_EVT_DESC_DISC_RSP:
    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
    case BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP:
    case BLE_GATTC_EVT_WRITE_RSP:
    case BLE_GATTC_EVT_HVX:
    case BLE_GATTS_EVT_WRITE:
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
    nrf52_ppogatt_bond_complete();
    _bt_conn_is_bonded = 1;
}

void hw_bluetooth_bond_acknowledge(int accepted) {
    if (!accepted)
        return; /* disconnect? */
    
    assert(!_bt_conn_is_bonded);
    _bt_conn_is_bonded = 1;
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "bond accepted!");
    nrf52_ppogatt_bond_complete();
}

static void _ble_disc_handler(ble_db_discovery_evt_t *evt) {
    nrf52_ppogatt_discovery(evt);
    
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
        int is_gap = (evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE) && (evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_GAP);
        if (!is_gap)
            return;
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: GAP, %d chars",
            evt->params.discovered_db.char_count);
        
        for (int i = 0; i < evt->params.discovered_db.char_count; i++) {
            const ble_gatt_db_char_t *dbchar = &(evt->params.discovered_db.charateristics[i]); /* Yes, the typo is in the nRF SDK. */
            
            if (dbchar->characteristic.uuid.uuid == BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME) {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: found device name characteristic w/ value handle %04x", dbchar->characteristic.handle_value);
                _bt_remote_name_hnd = dbchar->characteristic.handle_value;
                if (_bt_remote_name_request_desired /* i.e., we came in from a pairing request */) {
                    _bt_remote_name_request_queued = 1;
                    _enqueue_remote_name_request();
                }
            } else {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: other characteristic uuid %04x value handle %04x, cccd handle %04x", dbchar->characteristic.uuid.uuid, dbchar->characteristic.handle_value, dbchar->cccd_handle);
            }
        }
        break;
    }
    }
}

void bt_set_callback_get_bond_data(bt_callback_get_bond_data_t cbk) {
    _bt_get_bond_data = cbk;
}

void bt_set_callback_request_bond(bt_callback_request_bond_t cbk) {
    _bt_request_bond = cbk;
}

static void _pairing_init() {
    ret_code_t rv;
    
    NRF_SDH_BLE_OBSERVER(pairing_observer, 3 /* priority */, _pairing_handler, NULL);

    /* Listen for the GAP UUID, so we can discover the device name. */
    ble_uuid_t gap_uuid;
    gap_uuid.type = BLE_UUID_TYPE_BLE;
    gap_uuid.uuid = BLE_UUID_GAP;
    
    ble_db_discovery_init_t db_init = {};
    db_init.evt_handler  = _ble_disc_handler;
    db_init.p_gatt_queue = &_gatt_queue;

    rv = ble_db_discovery_init(&db_init);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_init");

    rv = ble_db_discovery_evt_register(&gap_uuid);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_evt_register(GAP)");
}

