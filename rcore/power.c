/* power.c
 * routines for controlling the battery and charge data
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include <stdbool.h>
#include "power.h"
#include "rebbleos.h"
#include "notification_manager.h"
#include "battery_state_service.h"

static uint8_t _charge_mode_prev = 0;
static uint8_t _charge_mode = 0;
static uint16_t _bat_voltage = 0;
static uint8_t _bat_pct = 0;

void power_init()
{
    hw_power_init();
    power_update_battery();
}

void power_off()
{
}

uint8_t power_get_charge_mode(void)
{
    return _charge_mode;
}

uint16_t power_get_battery_voltage(void)
{
    return _bat_voltage;
}

uint8_t power_get_battery_level(void)
{
    return _bat_pct;
}

void power_update_charge_mode(void)
{
    uint8_t mode = hw_power_get_chg_status();
    if (mode == _charge_mode_prev)
        return;

    _charge_mode_prev = mode;
    _charge_mode = mode;
    
    /* Show the popup as something changed */
    notification_show_battery(5000);
    
    battery_state_service_state_change();
}

void power_update_battery(void)
{
    _bat_voltage = hw_power_get_bat_mv();
    _bat_pct = map_range(_bat_voltage, 2600, 3500, 0, 100);
    SYS_LOG("PWR", APP_LOG_LEVEL_INFO, "VBAT %ldmV %d%%", _bat_voltage, _bat_pct);
    
    /* battery low */
    if (_bat_pct > 20)
        return;
    
    battery_state_service_state_change();
    
    notification_show_battery(5000);
}
