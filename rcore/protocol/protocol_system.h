#pragma once
void process_version_packet(uint8_t *data);
void process_set_time_packet(uint8_t *data);

/* This isn't actually our version, this is a faked out version for Pebble
 * app to at least consider talking to us over bluetooth */
static const char * const FW_VERSION = "v3.4-RebbleOS";




/* Time Command functions */
typedef enum cmd_time_func {
    TIME_GETTIME = 0,
    TIME_SETTIME = 2,
    TIME_SETTIME_UTC = 3,
} cmd_time_func_t;

typedef struct {
    uint8_t cmd;
    uint8_t ts;
    uint8_t tso;
    uint8_t tz_len;
    uint8_t tz;
} __attribute__((__packed__)) cmd_set_time_t;

