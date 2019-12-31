/* protocol_music.c
 * R/Pebble Protocol for Music handling
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "rebbleos.h"
#include "protocol.h"
#include "pebble_protocol.h"

/* Configure Logging */
#define MODULE_NAME "p-,us"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

typedef struct music_message_t {
    uint8_t command_id;
    uint8_t data[];
}  __attribute__((__packed__)) music_message;

typedef struct rebble_music_track_t {
    uint8_t command_id;
    uint8_t *artist;
    uint8_t *album;
    uint8_t *title;
    uint32_t track_length;
    uint16_t track_count;
    uint16_t current_track;
} rebble_music_track;

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
            int len = 0;
            rebble_music_track track = {
                .command_id = msg->command_id,
            };
            int tlen = 0;
            len = pascal_string_to_string(msg->data, msg->data);
            track.artist = msg->data;
            tlen += len;
            len = pascal_string_to_string(msg->data + tlen, msg->data + tlen);
            track.album = msg->data + tlen;
            tlen += len;
            len = pascal_string_to_string(msg->data + tlen, msg->data + tlen);
            track.title = msg->data + tlen;

            /* These are optional apparently */
            //if (packet->length < len)
            //track_length, track_count, current_track
            LOG_INFO("Title: %s", track.title);
            LOG_INFO("Artist: %s", track.artist);
            LOG_INFO("Album: %s", track.album);
            break;
        case MusicMessage_UpdatePlayStateInfo:
            LOG_INFO("Play State");
            rebble_play_state *ps = (rebble_play_state *)msg;
            LOG_INFO("Play State S %x, P %d R %d Sh %x Rp %x", ps->state, ps->track_pos, ps->play_rate, ps->shuffle, ps->repeat);
            break;
        case MusicMessage_UpdateVolumeInfo:
            LOG_INFO("Volume Pct %d", msg->data[0]);
            break;
        case MusicMessage_UpdatePlayerInfo:
            len = pascal_string_to_string(msg->data, msg->data);
            uint8_t *package = msg->data;
            len = pascal_string_to_string(msg->data + len, msg->data + len);
            uint8_t *name = msg->data + len;
            LOG_INFO("Player %s %s", package, name);            
            break;
        default:
            LOG_ERROR("Unknown Command %d", msg->command_id);    
    }
}

inline void protocol_music_playpause()
{
    protocol_phone_message_send(MusicMessage_PlayPause);
}

inline void protocol_music_pause()
{
    protocol_phone_message_send(MusicMessage_Pause);
}

inline void protocol_music_play()
{
    protocol_phone_message_send(MusicMessage_Play);
}

inline void protocol_music_next()
{
    protocol_phone_message_send(MusicMessage_NextTrack);
}

inline void protocol_music_prev()
{
    protocol_phone_message_send(MusicMessage_PreviousTrack);
}

inline void protocol_music_volup()
{
    protocol_phone_message_send(MusicMessage_VolumeUp);
}

inline void protocol_music_voldown()
{
    protocol_phone_message_send(MusicMessage_VolumeDown);
}

inline void protocol_music_get_current_track()
{
    protocol_phone_message_send(MusicMessage_GetCurrentTrack);
}

void protocol_music_message_send(uint8_t command_id)
{
    music_message *mm = protocol_calloc(1, sizeof(music_message));
    mm->command_id = command_id;
    
    /* Send a phone action */
    rebble_protocol_send(WatchProtocol_PhoneMessage, NULL, mm, sizeof(music_message), 
                          3, 1500, false);
}

