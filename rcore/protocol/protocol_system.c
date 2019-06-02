#include <stdlib.h>
#include "rebbleos.h"
#include "protocol_system.h"
#include "endpoint.h"

// firmware version processing
void process_version_packet(uint8_t *data)
{
    switch(data[0])
    {
        case FIRMWARE_VERSION_GETVERSION:
            SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "Get Version");
            uint16_t tx_len = strlen(FW_VERSION);
            
            /* Reply back with our firmware version */
            bluetooth_send_data(ENDPOINT_FIRMWARE_VERSION, (uint8_t *)FW_VERSION, tx_len);
            break;
    }
}


void process_set_time_packet(uint8_t *data)
{
    cmd_set_time_t *time = (cmd_set_time_t *)data;
    SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "XXX Time Set cmd %d, ts %d tso %d, tz %d",
            time->cmd, time->ts, time->tso, time->tz);
}
