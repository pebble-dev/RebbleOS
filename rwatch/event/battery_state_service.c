/* routines that implement the PebbleOS battery api
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 *
 */

#include "librebble.h"
#include "ngfxwrap.h"
#include "battery_state_service.h"
#include "power.h"

static BatteryStateHandler _state_handler;

BatteryChargeState battery_state_service_peek(void)
{
    uint8_t mode = power_get_charge_mode();
    BatteryChargeState state = {
        .charge_percent = power_get_battery_level(),
        .is_charging =  mode > 1 && mode != POWER_CHG_FAULT,
        .is_plugged = mode == POWER_CHG_MODE_MAINT_CHG_DONE
    };
    SYS_LOG("BATT", APP_LOG_LEVEL_INFO, "Chg %d", state.charge_percent);
    
    return state;
}

void battery_state_service_subscribe(BatteryStateHandler handler)
{
    _state_handler = handler;
}

void battery_state_service_unsubscribe(void)
{
    _state_handler = NULL;
}

void battery_state_service_state_change(void)
{
    SYS_LOG("BATT", APP_LOG_LEVEL_INFO, "Update");
    if (_state_handler != NULL)
        _state_handler(battery_state_service_peek());
}
