#pragma once
/* power.h
 * routines for controlling the battery and charge data
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"

/**
 * @brief Charging modes. Values 2 -> 6 are "charging"
 */
#define POWER_CHG_MODE_OFF              0
#define POWER_CHG_MODE_OVER_TEMP        1
#define POWER_CHG_MODE_PRE_CHG          2
#define POWER_CHG_MODE_FAST_CHG_CC      3
#define POWER_CHG_MODE_FAST_CHG_CV      4
#define POWER_CHG_MODE_MAINT_CHG        5
#define POWER_CHG_MODE_MAINT_CHG_DONE   6
#define POWER_CHG_FAULT                 7

/**
 * @brief Initiailise the battery PMIC module
 */
void power_init(void);

/**
 * @brief De-Initiailise the battery PMIC module
 */
void power_off(void);

/**
 * @brief Get the value of the current charging mode
 * @return value of the current charge mode
 */
uint8_t power_get_charge_mode(void);

/**
 * @brief Get the current voltage
 * @return Voltage in mV
 */
uint16_t power_get_battery_voltage(void);

/**
 * @brief Get the battery level 0 - 100.
 * Minor filtering done
 * @return unrounded battery voltage
 */
uint8_t power_get_battery_level(void);

/**
 * @brief Update the charge mode of the power unit
 * Will interrogate any PMIC for value and update registers
 */
void power_update_charge_mode(void);

/**
 * @brief Get the value of the battery
 */
void power_update_battery(void);
