#pragma once
#include "endpoint.h"
#include "protocol_notification.h"
#include "protocol_system.h"
#include "protocol_call.h"
#include "protocol_music.h"

enum {
    WatchProtocol_Time               = 0x000b,
    WatchProtocol_FirmwareVersion    = 0x0010, /* Versions, in Pebble app */
    WatchProtocol_AppVersion         = 0x0011,
    WatchProtocol_SystemMessage      = 0x0012,
    WatchProtocol_MusicControl       = 0x0020,
    WatchProtocol_PhoneMessage       = 0x0021,
    WatchProtocol_AppMessage         = 48,
    WatchProtocol_AppCustomize       = 50,
    WatchProtocol_AppRunState        = 52,
    WatchProtocol_Logs               = 2000,
    WatchProtocol_PingPong           = 2001,
    WatchProtocol_LogDump            = 2002, 
    WatchProtocol_Reset              = 2003,
    WatchProtocol_AppLogs            = 2006,
    WatchProtocol_SysReg             = 5000, /* ??? */
    WatchProtocol_WatchModel         = 5001, /* FctReg, in Pebble */
    WatchProtocol_AppFetch           = 0x1771,
    WatchProtocol_PutBytes           = 0xBEEF,
    WatchProtocol_DataLog            = 6778,
    WatchProtocol_Screenshot         = 8000,
    WatchProtocol_FileInstallManager = 8181,
    WatchProtocol_GetBytes           = 9000,
    WatchProtocol_AudioStreaming     = 10000,
    WatchProtocol_AppReorder         = 0xabcd,
    WatchProtocol_LegacyMessage      = 0x0bc2, // legacy. Yes
    WatchProtocol_BlobDbMessage      = 0xb1db,
    WatchProtocol_BlobDbV2Message    = 0xb2db,
    WatchProtocol_TimelineAction     = 11440,
    WatchProtocol_VoiceControl       = 11000,
    WatchProtocol_HealthSync         = 911,
};


