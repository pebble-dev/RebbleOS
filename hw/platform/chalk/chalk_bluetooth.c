#include "rebbleos.h"

/*
 * NOT IMPLEMENTED
 * Stubs for features not yet enabled on this platform
 */

void hw_bluetooth_init() {
    rebbleos_module_set_status(MODULE_BLUETOOTH, MODULE_DISABLED, MODULE_ERROR);
}

void bt_device_request_tx() {
}
