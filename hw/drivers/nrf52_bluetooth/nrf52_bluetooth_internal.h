/* nrf52_bluetooth_internal.h
 * Definitions for internal components of nRF52 Bluetooth driver, including PPoGATT.  Not for external use.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */
#pragma once

#include "ble_db_discovery.h"

#define PPOGATT_MTU 256

/* nrf52_bluetooth_ppogatt.c */
void nrf52_ppogatt_bond_complete();
void nrf52_ppogatt_discovery(ble_db_discovery_evt_t *evt);
void nrf52_ppogatt_init();
