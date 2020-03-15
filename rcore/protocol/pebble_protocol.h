#pragma once
#include "endpoint.h"
#include "protocol_notification.h"
#include "protocol_system.h"
#include "protocol_call.h"
#include "protocol_music.h"

enum {
    WatchProtocol_Time             = 0x000b,
    WatchProtocol_FirmwareVersion  = 0x0010,
    WatchProtocol_AppVersion       = 0x0011,
    WatchProtocol_SystemMessage    = 0x0012,
    WatchProtocol_MusicControl     = 0x0020,
    WatchProtocol_PhoneMessage     = 0x0021,
    WatchProtocol_AppRunState      = 0x34,
    WatchProtocol_AppFetch         = 0x1771,
    WatchProtocol_WatchModel       = 5001,
    WatchProtocol_PingPong         = 2001, 
    WatchProtocol_Reset            = 2003,
    WatchProtocol_LegacyMessage    = 0x0bc2, // legacy. Yes
    WatchProtocol_BlobDbMessage    = 0xb1db,
    WatchProtocol_TimelineAction   = 11440,
    WatchProtocol_PutBytes         = 0xBEEF,
};


