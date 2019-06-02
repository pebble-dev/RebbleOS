#pragma once
#include "endpoint.h"
#include "protocol_notification.h"
#include "protocol_system.h"

enum {
    WatchProtocol_Time             = 0x000b,
    WatchProtocol_FirmwareVersion  = 0x0010,
    WatchProtocol_AppVersion       = 0x0011,
    WatchProtocol_SystemMessage    = 0x0012, // not connected yet
    WatchProtocol_AppRunState      = 0x34,
    WatchProtocol_AppFetch         = 0x1771,
    WatchProtocol_WatchModel       = 5001, //5001
    WatchProtocol_PingPong         = 2001, 
    WatchProtocol_Reset            = 2003,
    WatchProtocol_PhoneMessage     = 0x0bc2, // legacy?
    WatchProtocol_BlobDbMessage    = 0xb1db,
};


