/* nrf52_bluetooth_ppogatt.c
 * PPoGATT discovery implementation for nRF52
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */
#define BLUETOOTH_IS_BLE 1
#include <debug.h>
#include "log.h"
#include "rbl_bluetooth.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_srv_common.h"
#include "ble_db_discovery.h"
#include "nrf52_bluetooth_internal.h"

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

static int _ppogatt_is_ready = 0;

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

static uint8_t ppogatt_srv_wr_buf[PPOGATT_MTU];
static uint8_t ppogatt_srv_rd_buf[PPOGATT_MTU];

static uint16_t pebble_metadata_srv_svc_hnd = BLE_GATT_HANDLE_INVALID;
static uint8_t pebble_metadata_connectivity_buf[4] = { 0x1f, 0x00, 0x00, 0x00 };
static uint16_t pebble_metadata_connectivity_cccd = 0;
static ble_gatts_char_handles_t pebble_metadata_srv_connectivity_hnd;
static uint8_t pebble_metadata_pairing_buf[1] = { 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_pairing_hnd;
static uint8_t pebble_metadata_mtu_buf[2] = { 0x80, 0x00 };
static ble_gatts_char_handles_t pebble_metadata_srv_mtu_hnd;
static uint8_t pebble_metadata_parameters_buf[7] = { 0x00, 0x9c, 0x00, 0x00, 0x00, 0x58, 0x02 };
static ble_gatts_char_handles_t pebble_metadata_srv_parameters_hnd;

static ble_ppogatt_callback_connected_t _ppogatt_callback_connected;
static ble_ppogatt_callback_txready_t _ppogatt_callback_txready;
static ble_ppogatt_callback_rx_t _ppogatt_callback_rx;
static ble_ppogatt_callback_disconnected_t _ppogatt_callback_disconnected;

static int _ppogatt_bond_complete = 0;
static int _ppogatt_connectivity_updated = 0;

static void _hvx_connectivity_update() {
    if (pebble_metadata_connectivity_cccd) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "doing HVX for connectivity");
        
        ble_gatts_hvx_params_t hvx;
        uint16_t len16 = sizeof(pebble_metadata_connectivity_buf);
    
        memset(&hvx, 0, sizeof(hvx));
        hvx.type = BLE_GATT_HVX_NOTIFICATION;
        hvx.handle = pebble_metadata_srv_connectivity_hnd.value_handle;
        hvx.p_data = pebble_metadata_connectivity_buf;
        hvx.p_len = &len16;
     
        ret_code_t rv = sd_ble_gatts_hvx(_bt_conn, &hvx);
        if (rv != NRF_SUCCESS) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "failed to HVX for connectivity");
        }
        
        _ppogatt_connectivity_updated = 1;
    }
}

static void _update_connection() {
    if (_ppogatt_bond_complete && _ppogatt_is_ready && !_ppogatt_connectivity_updated)
        _hvx_connectivity_update();
    if (_ppogatt_bond_complete && _ppogatt_is_ready && _ppogatt_connectivity_updated)
        _ppogatt_callback_connected();
}

void nrf52_ppogatt_bond_complete() {
    assert(!_ppogatt_bond_complete);
    _ppogatt_bond_complete = 1;
    if (_ppogatt_is_ready) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT: connected, because the bond is complete");
        _update_connection();
    }
}

static void _ppogatt_handler(const ble_evt_t *evt, void *context) {
    ret_code_t rv;

    switch (evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        ppogatt_srv_notify_cccd = 0;
        pebble_metadata_connectivity_cccd = 0;
        _ppogatt_is_ready = 0;
        _ppogatt_connectivity_updated = 0;
        _ppogatt_bond_complete = 0;
        _bt_conn = evt->evt.gap_evt.conn_handle;
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        ppogatt_srv_notify_cccd = 0;
        ppogatt_cli_data_value_hnd = BLE_GATT_HANDLE_INVALID;
        ppogatt_cli_data_cccd_hnd = BLE_GATT_HANDLE_INVALID;
        _ppogatt_callback_disconnected();
        _bt_conn = BLE_CONN_HANDLE_INVALID;
        break;
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
        if (evtwr->handle != ppogatt_srv_write_hnd.value_handle && evtwr->len >= 2)
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "GATTS write evt: handle %04x %s, op %02x, auth req %u, offset %02x, len %d (%02x %02x)",
                evtwr->handle, hname, evtwr->op, evtwr->auth_required, evtwr->offset, evtwr->len,
                evtwr->data[0], evtwr->data[1]);
        else
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "GATTS write evt: handle %04x %s, op %02x, auth req %u, offset %02x, len %d", evtwr->handle, hname, evtwr->op, evtwr->auth_required, evtwr->offset, evtwr->len);
        
        if (evtwr->handle == ppogatt_srv_write_hnd.value_handle && _ppogatt_bond_complete) {
            _ppogatt_callback_rx(evtwr->data, evtwr->len);
        } else if (evtwr->handle == ppogatt_srv_notify_hnd.cccd_handle && evtwr->len <= 2) {
            memcpy(&ppogatt_srv_notify_cccd, evtwr->data, evtwr->len);
            if (ppogatt_srv_notify_cccd) {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT: ready to connect, because the remote end set up HVX listens");
                _ppogatt_is_ready = 1;
                if (!_ppogatt_is_ready && _ppogatt_bond_complete) {
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT: and firing off connected callback");
                    _update_connection();
                }
            }
        } else if (evtwr->handle == pebble_metadata_srv_connectivity_hnd.cccd_handle && evtwr->len <= 2) {
            memcpy(&pebble_metadata_connectivity_cccd, evtwr->data, evtwr->len);
            if (_ppogatt_bond_complete)
                _update_connection();
        }
        break;
    }
    case BLE_GATTC_EVT_HVX: {
        const ble_gattc_evt_hvx_t *evthvx = &evt->evt.gattc_evt.params.hvx;

        if (evthvx->handle != ppogatt_cli_data_value_hnd) {
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE_GATTC_EVT_HVX: unexpected handle %04x (hoping for %04x)?", evthvx->handle, ppogatt_cli_data_value_hnd);
            break;
        }
        
        if (_ppogatt_bond_complete)
            _ppogatt_callback_rx(evthvx->data, evthvx->len);
        break;
    }
    case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
    case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        _ppogatt_callback_txready();
        break;
    }
}

void nrf52_ppogatt_discovery(ble_db_discovery_evt_t *evt) {
    switch (evt->evt_type) {
    case BLE_DB_DISCOVERY_COMPLETE: {
        int is_ppogatt = evt->params.discovered_db.srv_uuid.type == ppogatt_cli_svc_uuid.type;
        if (!is_ppogatt)
            return;
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: PPoGATT, %d chars",
            evt->params.discovered_db.char_count);
        
        for (int i = 0; i < evt->params.discovered_db.char_count; i++) {
            const ble_gatt_db_char_t *dbchar = &(evt->params.discovered_db.charateristics[i]); /* Yes, the typo is in the nRF SDK. */
            
            if (dbchar->characteristic.uuid.uuid == 0x0001) {
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
                
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT: ready to connect, because we found the PPoGATT server characteristic");
                _ppogatt_is_ready = 1;
                if (!_ppogatt_is_ready && _ppogatt_bond_complete) {
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT: and firing off connected callback");
                    _update_connection();
                }
            } else {
                DRV_LOG("bt", APP_LOG_LEVEL_INFO, "BLE remote service discovery: other characteristic uuid %04x value handle %04x, cccd handle %04x", dbchar->characteristic.uuid.uuid, dbchar->characteristic.handle_value, dbchar->cccd_handle);
            }
        }
        break;
    }
    default:
        break;
    }
}

/* On-boot PPoGATT setup (adds GATT server on watch side). */
void nrf52_ppogatt_init() {
    ret_code_t rv;
    /* UUID: 30000003-328e-0fbb-c642-1aa6699bdada */
    /*   characteristics: 0004, 0005, 0006 */

    /* Add event observer for PPoGATT services. */	
    NRF_SDH_BLE_OBSERVER(ppogatt_observer, 3 /* priority */, _ppogatt_handler, NULL);
    
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
    

    rv = ble_db_discovery_evt_register(&ppogatt_cli_svc_uuid);
    assert(rv == NRF_SUCCESS && "ble_db_discovery_evt_register(PPoGATT)");
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
