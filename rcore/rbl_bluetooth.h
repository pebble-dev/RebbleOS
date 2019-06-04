#pragma once
#include "stdbool.h"
#include "platform.h"

#define TX_BUFFER_SIZE 250
#define TX_TIMEOUT_MS 5

typedef struct pbl_transport_packet_t {
    uint16_t length;
    uint16_t endpoint;
    uint8_t *data;
} __attribute__((__packed__)) pbl_transport_packet;



typedef void (*tx_complete_callback)();

uint8_t bluetooth_init(void);
void bluetooth_init_complete(uint8_t state);
void bluetooth_data_rx_notify(size_t len);
uint32_t bluetooth_send_serial_raw(uint8_t *data, size_t len);
uint8_t bluetooth_send(uint8_t *data, size_t len);
void bluetooth_send_packet(uint16_t endpoint, uint8_t *data, uint16_t len);
uint32_t bluetooth_tx_buf_get_bytes(uint8_t *data, size_t len);
void bluetooth_data_rx(uint8_t *data, size_t len);
void bluetooth_tx_complete_from_isr(void);


uint8_t hw_bluetooth_power_cycle(void);
void hw_bluetooth_enable_cts_irq(void);
void hw_bluetooth_disable_cts_irq(void);
uint8_t hw_bluetooth_init(void);


void bluetooth_device_connected(void);
void bluetooth_device_disconnected(void);
bool bluetooth_is_device_connected(void);

#ifdef BLUETOOTH_IS_BLE
void ppogatt_init(void);

/* BLE stack <-> PPoGATT module communications */

typedef void (*ble_ppogatt_callback_txready_t)();
typedef void (*ble_ppogatt_callback_rx_t)(const uint8_t *buf, size_t len);

extern int ble_ppogatt_tx(const uint8_t *buf, size_t len);
extern void ble_ppogatt_set_callback_txready(ble_ppogatt_callback_txready_t cbk);
extern void ble_ppogatt_set_callback_rx(ble_ppogatt_callback_rx_t cbk);

#endif
