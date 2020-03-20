#include "rbl_bluetooth.h"
#include "init.h"

/*
 * NOT IMPLEMENTED
 * Stubs for features not yet enabled on this platform
 */

uint8_t hw_bluetooth_init() {
    bluetooth_init_complete(INIT_RESP_OK);
    return 0;
}

void bt_device_request_tx() {
}
