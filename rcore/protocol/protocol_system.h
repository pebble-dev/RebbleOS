#pragma once
#include "protocol.h"

void protocol_watch_version(const RebblePacket packet);
void protocol_watch_model(const RebblePacket packet);
void protocol_ping_pong(const RebblePacket packet);
void protocol_watch_reset(const RebblePacket packet);
void protocol_app_version(const RebblePacket packet);
void protocol_time(const RebblePacket packet);

/* This isn't actually our version, this is a faked out version for Pebble
 * app to at least consider talking to us over bluetooth */
static const char * const FW_VERSION = "v3.4.5-2RebbleOS-2";


