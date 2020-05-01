#pragma once
typedef struct rebble_music_track_t {
    uint8_t command_id;
    uint8_t *artist;
    uint8_t *album;
    uint8_t *title;
    uint32_t track_length;
    uint16_t track_count;
    uint16_t current_track;
} rebble_music_track;

typedef rebble_music_track MusicTrackInfo;

void protocol_music_message_process(const RebblePacket packet);
MusicTrackInfo *protocol_music_decode(RebblePacket packet);
void protocol_music_destroy(RebblePacket packet);

void protocol_music_playpause();
void protocol_music_pause();
void protocol_music_play();
void protocol_music_next();
void protocol_music_prev();
void protocol_music_volup();
void protocol_music_voldown();
void protocol_music_message_send(uint8_t command_id);
void protocol_music_get_current_track();