#pragma once
#include "stdbool.h"
#include "platform.h"
#include <stddef.h>

#define TX_TIMEOUT_MS 10


typedef void (*tx_complete_callback)();

uint8_t bluetooth_init(void);
void bluetooth_init_complete(uint8_t state);
void bluetooth_data_rx_notify(size_t len);
uint32_t bluetooth_send_serial_raw(uint8_t *data, size_t len);
uint8_t bluetooth_send(uint8_t *data, size_t len);
void bluetooth_send_data(uint16_t endpoint, uint8_t *data, uint16_t len);
uint32_t bluetooth_tx_buf_get_bytes(uint8_t *data, size_t len);
void bluetooth_data_rx(uint8_t *data, size_t len);
void bluetooth_tx_complete_from_isr(void);


uint8_t hw_bluetooth_power_cycle(void);
void hw_bluetooth_enable_cts_irq(void);
void hw_bluetooth_disable_cts_irq(void);
uint8_t hw_bluetooth_init(void);
const char *hw_bluetooth_name();
void hw_bluetooth_advertising_visible(int vis);

void bluetooth_device_connected(void);
void bluetooth_device_disconnected(void);
bool bluetooth_is_device_connected(void);
void bluetooth_enable(void);
bool bluetooth_is_enabled(void);
void bluetooth_bond_acknowledge(int accepted);
void bluetooth_advertising_visible(int visible);
const char *bluetooth_name();

/* Bonding database access.
 *
 * These callbacks come from ISR context, and should return quickly, and
 * certainly must not block (for instance, as if they were accessing a
 * database); functionally, this means pushing the requests onto a queue for
 * a thread to handle.
 *
 * The expected sequence is, approximately, as follows:
 *
 *  * When a Bluetooth central connects to a Rebble device, the stack calls
 *    into the get_bond_data callback with an opaque "peer" (which may or
 *    may not match a MAC address).  If the database contains bond data for
 *    that "peer", the opaque bond data is (later, not on IRQ context)
 *    returned in bond_data_available.  If it isn't, then
 *    bond_data_available is called with a NULL data.
 *
 *  * Later, if new bond data are created, then the stack calls into the
 *    request_bond callback with the name of the peer and with the opaque
 *    data to be returned next time.  If the UI agrees to bond with this
 *    device, it stores the bond data, then calls back bond_acknowledge(1);
 *    otherwise, it calls back bond_acknowledge(0) (or simply does nothing).
 */
typedef void (*bt_callback_get_bond_data_t)(const void *peer, size_t len);
typedef void (*bt_callback_request_bond_t)(const void *peer, size_t len, const char *name, const void *data, size_t datalen);

extern void bt_set_callback_get_bond_data(bt_callback_get_bond_data_t cbk);
extern void bt_set_callback_request_bond(bt_callback_request_bond_t cbk);
extern void hw_bluetooth_bond_data_available(const void *data, size_t datalen);
extern void hw_bluetooth_bond_acknowledge(int accepted);

#ifdef BLUETOOTH_IS_BLE
void ppogatt_init(void);

/* BLE stack <-> PPoGATT module communications */

typedef void (*ble_ppogatt_callback_connected_t)();
typedef void (*ble_ppogatt_callback_txready_t)();
typedef void (*ble_ppogatt_callback_rx_t)(const uint8_t *buf, size_t len);
typedef void (*ble_ppogatt_callback_disconnected_t)();

extern int ble_ppogatt_tx(const uint8_t *buf, size_t len);
extern void ble_ppogatt_set_callback_connected(ble_ppogatt_callback_connected_t cbk);
extern void ble_ppogatt_set_callback_txready(ble_ppogatt_callback_txready_t cbk);
extern void ble_ppogatt_set_callback_rx(ble_ppogatt_callback_rx_t cbk);
extern void ble_ppogatt_set_callback_disconnected(ble_ppogatt_callback_disconnected_t cbk);

#endif

#define INIT_RESP_OK            0
#define INIT_RESP_ASYNC_WAIT    1
#define INIT_RESP_NOT_SUPPORTED 2
#define INIT_RESP_ERROR         3