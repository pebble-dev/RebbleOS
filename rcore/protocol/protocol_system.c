#include "rebbleos.h"
#include "protocol.h"
#include "protocol_system.h"
#include "pebble_protocol.h"

typedef struct firmware_version_t {
    uint32_t timestamp;
    uint8_t version_tag[32];
    uint8_t git_hash[8];
    uint8_t is_recovery;
    uint8_t hardware_platform;
    uint8_t metadata_version;
}  __attribute__((__packed__)) firmware_version;

typedef struct firmware_version_response_t {
    uint8_t command;
    const struct firmware_version_t running;
    const struct firmware_version_t recovery;
    uint32_t bootloader_timestamp;
    uint8_t board[9];
    uint8_t serial[12];
    uint8_t bt_address[6];
    uint32_t resource_crc;
    uint32_t resource_timestamp;
    uint8_t language[6];
    uint16_t language_version;
    uint64_t capabilities; // = Uint64(endianness='<')
    uint8_t is_unfaithful;// = Optional(Boolean())
}  __attribute__((__packed__)) firmware_version_response;

// firmware version processing
void protocol_watch_version(const pbl_transport_packet *packet)
{
    if (((uint8_t *)packet->data)[0] != 0)
    {
        SYS_LOG("FWPKT", APP_LOG_LEVEL_ERROR, "Not a request");
        return;
    }

    SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "Get Version");
    uint16_t tx_len = strlen(FW_VERSION);

    const firmware_version_response response = (firmware_version_response) {
            .command = 1,
            .running = (firmware_version) {
                    .timestamp = 1479829116,
                    .version_tag = "v4.3",
                    .git_hash = {0x30,0x65,0x33,0x63,0x33,0x66,0x64 },// {0x0,0xe,0x3,0xc,0xf,0xd},
                    .is_recovery = 0,
                    .hardware_platform = 252,
                    .metadata_version = 1
                },
            .recovery = (firmware_version) {
                    .timestamp = 1479829116,
                    .version_tag = "v4.3",
                    .git_hash = {0x30,0x65,0x33,0x63,0x33,0x66,0x64 },// {0x0,0xe,0x3,0xc,0xf,0xd},
                    .is_recovery = 0,
                    .hardware_platform = 252,
                    .metadata_version = 1
                },
            .bootloader_timestamp = get_boot_tick(),
            .board = {0},
            .serial = {0},
            .bt_address = {0},
            .resource_crc = 3969437875,
            .resource_timestamp = 0,
            .language = "en_US",
            .language_version = ntohs(1),
            .capabilities = 2180863, // = Uint64(endianness='<')
            .is_unfaithful = 1, // = Optional(Boolean())
        };

    /* Reply back with our firmware version */
    pbl_transport_packet pkt = {
            .length = sizeof(firmware_version_response),
            .endpoint = WatchProtocol_FirmwareVersion,
            .data = (uint8_t *)&response,
            .transport_sender = packet->transport_sender
        };


    protocol_send_packet(&pkt);
}

void protocol_watch_model(const pbl_transport_packet *packet)
{
    uint8_t resp[6] = { 1, 4, 0, 0, 0, SnowyBlack };
    pbl_transport_packet pkt = {
        .length = 6,
        .endpoint = WatchProtocol_WatchModel,
        .data = (uint8_t *)resp,
        .transport_sender = packet->transport_sender
    };
    protocol_send_packet(&pkt);
}

void protocol_ping_pong(const pbl_transport_packet *packet)
{
    uint8_t resp[5] = { 1, packet->data[1], packet->data[2], packet->data[3], packet->data[4] };
    pbl_transport_packet pkt = {
        .length = 5,
        .endpoint = WatchProtocol_PingPong,
        .data = (uint8_t *)resp,
        .transport_sender = packet->transport_sender
    };
    protocol_send_packet(&pkt);
}

void protocol_watch_reset(const pbl_transport_packet *packet)
{
   assert(0);
}

typedef struct app_version_response_t {
    uint8_t command;
    uint32_t protocol_version; // = Uint32()  # Unused as of v3.0
    uint32_t session_caps; // = Uint32()  # Unused as of v3.0
    uint32_t platform_flags;
    uint8_t response_version; // = Uint8(default=2)
    uint8_t major_version;
    uint8_t minor_version;
    uint8_t bugfix_version;
    uint64_t protocol_caps;
} __attribute__((__packed__)) app_version_response;

void protocol_app_version(const pbl_transport_packet *packet)
{
    app_version_response appv = {
        .command = 1,
        .protocol_version = 0, // = Uint32()  # Unused as of v3.0
        .session_caps = 0, // = Uint32()  # Unused as of v3.0
        .platform_flags = 0,
        .response_version = 2, // = Uint8(default=2)
        .major_version = 0,
        .minor_version = 0,
        .bugfix_version = 0,
        .protocol_caps = 0,
    };

    pbl_transport_packet pkt = {
        .length = sizeof(app_version_response),
        .endpoint = WatchProtocol_AppVersion,
        .data = (uint8_t *)&appv,
        .transport_sender = packet->transport_sender
    };
    protocol_send_packet(&pkt);
}

/* Time Command functions */
enum {
    GetTimeRequest,
    GetTimeResponse,
    SetLocalTime,
    SetUTC
};

typedef struct cmd_set_time_utc_t {
    uint8_t command;
    uint32_t unix_time;
    int16_t utc_offset;
    uint8_t pstr_len;
    char tz_name[];
} __attribute__((__packed__)) cmd_set_time_utc;

extern struct tm *boot_time_tm;
void protocol_time(const pbl_transport_packet *packet)
{
    uint8_t buf[5];
    pbl_transport_packet pkt = {
            .length = 5,
            .endpoint = WatchProtocol_Time,
            .data = buf,
            .transport_sender = packet->transport_sender
        };

    if (packet->data[0] == GetTimeRequest)
    {
        buf[0] = GetTimeResponse;
        uint32_t t = ntohl(rcore_get_time()); //pbl_time_t_deprecated(NULL);
        memcpy(&buf[1], &t, 4);
        SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "XXX Time %d", htonl(t));
        protocol_send_packet(&pkt);
    }
    else if (packet->data[0] == SetLocalTime)
    {
        uint32_t nt;
        memcpy(&nt, &packet->data[1], 4);
        SYS_LOG("FWPKT", APP_LOG_LEVEL_INFO, "XXX aTime %d %d", nt, ntohl(nt));
        rcore_set_time(ntohl(nt));
    }
    else if (packet->data[0] == SetUTC)
    {
        cmd_set_time_utc *utc = (cmd_set_time_utc *)&packet->data[1];
        rcore_set_time(ntohl(utc->unix_time));
        rcore_set_utc_offset(utc->utc_offset);
        rcore_set_tz_name(utc->tz_name, utc->pstr_len);
    }
    else
        assert(!"Invalid time request!");
}
