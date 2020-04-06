/* protocol_music.c
 * R/Pebble Protocol for Music handling
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol.h"
#include "pebble_protocol.h"
#include "protocol_service.h"
#include "event_service.h"

/* Configure Logging */
#define MODULE_NAME "p-,us"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

typedef struct music_message_t {
    uint8_t command_id;
    uint8_t data[];
}  __attribute__((__packed__)) music_message;



typedef struct rebble_play_state_t {
    uint8_t command_id;
    uint8_t state;
    uint32_t track_pos;
    uint32_t play_rate;
    uint8_t shuffle;
    uint8_t repeat;
} rebble_play_state;

enum {
    MusicMessage_PlayPause          = 0x01,
    MusicMessage_Pause              = 0x02,
    MusicMessage_Play               = 0x03,
    MusicMessage_NextTrack          = 0x04,
    MusicMessage_PreviousTrack      = 0x05,
    MusicMessage_VolumeUp           = 0x06,
    MusicMessage_VolumeDown         = 0x07,
    MusicMessage_GetCurrentTrack    = 0x08,
    MusicMessage_UpdateCurrentTrack = 0x10,
    MusicMessage_UpdatePlayStateInfo= 0x11,
    MusicMessage_UpdateVolumeInfo   = 0x12,
    MusicMessage_UpdatePlayerInfo   = 0x13,
};

enum {
    MusicMessageState_Paused,
    MusicMessageState_Playing,
    MusicMessageState_Rewinding,
    MusicMessageState_Fastforwarding,
    MusicMessageState_Unknown    
};

enum {
    MusicMessageShuffle_Unknown,
    MusicMessageShuffle_Off,
    MusicMessageShuffle_On,
};

enum {
    MusicMessageRepeat_Unknown,
    MusicMessageRepeat_Off,
    MusicMessageRepeat_One,
    MusicMessageRepeat_All,
};

void protocol_music_message_process(const RebblePacket packet)
{
    music_message *msg = (music_message *)packet_get_data(packet);
    
    LOG_DEBUG("M Message l %d", packet_get_data_length(packet));
    
    switch(msg->command_id)
    {
        case MusicMessage_UpdateCurrentTrack:
            LOG_INFO("Track");
            event_service_post(EventServiceCommandMusic, (void *)packet, (void *)protocol_music_destroy);
            break;
        case MusicMessage_UpdatePlayStateInfo:
            LOG_INFO("Play State");
            rebble_play_state *ps = (rebble_play_state *)msg->data;
            LOG_INFO("Play State S %x, P %d R %d Sh %x Rp %x", ps->state, ps->track_pos, ps->play_rate, ps->shuffle, ps->repeat);
            break;
        case MusicMessage_UpdateVolumeInfo:
            LOG_INFO("Volume Pct %d", msg->data[0]);
            break;
        case MusicMessage_UpdatePlayerInfo:
            LOG_INFO("Player");
            uint16_t len = pascal_string_to_string(msg->data, msg->data);
            uint8_t *package = msg->data;
            len = pascal_string_to_string(msg->data + len, msg->data + len);
            uint8_t *name = msg->data + len;
            LOG_INFO("Player %s %s", package, name);
            break;
        default:
            LOG_ERROR("Unknown Command %d", msg->command_id);    
    }   
}

MusicTrackInfo *protocol_music_decode(RebblePacket packet)
{
    int len = 0;
    int tlen = 0;
    
    music_message *msg = (music_message *)packet_get_data(packet);
    MusicTrackInfo *music = app_calloc(1, sizeof(MusicTrackInfo));

    music->command_id = msg->command_id;
    
    len = pascal_strlen((char *)msg->data);
    music->artist = app_calloc(1, len + 1);
    tlen = pascal_string_to_string(music->artist, msg->data);
    
    len = pascal_strlen((char *)(msg->data + tlen));
    music->album = app_calloc(1, len + 1);
    len = pascal_string_to_string(music->album, msg->data + tlen);
    tlen += len;
    
    len = pascal_strlen((char *)(msg->data + tlen));
    music->title = app_calloc(1, len + 1);
    len = pascal_string_to_string(music->title, msg->data + tlen);
    
    return music;
}

void protocol_music_destroy(RebblePacket packet)
{

}

inline void protocol_music_playpause()
{
    protocol_music_message_send(MusicMessage_PlayPause);
}

inline void protocol_music_pause()
{
    protocol_music_message_send(MusicMessage_Pause);
}

inline void protocol_music_play()
{
    protocol_music_message_send(MusicMessage_Play);
}

inline void protocol_music_next()
{
    protocol_music_message_send(MusicMessage_NextTrack);
}

inline void protocol_music_prev()
{
    protocol_music_message_send(MusicMessage_PreviousTrack);
}

inline void protocol_music_volup()
{
    protocol_music_message_send(MusicMessage_VolumeUp);
}

inline void protocol_music_voldown()
{
    protocol_music_message_send(MusicMessage_VolumeDown);
}

void protocol_music_get_current_track()
{
    protocol_music_message_send(MusicMessage_GetCurrentTrack);
}

void protocol_music_message_send(uint8_t command_id)
{
    RebblePacket packet = packet_create(WatchProtocol_MusicControl, sizeof(music_message));
    music_message *mm = (music_message *)packet_get_data(packet);
    mm->command_id = command_id;

    /* Send a phone action */
    packet_send(packet);
}

