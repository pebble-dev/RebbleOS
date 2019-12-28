void protocol_music_message_process(const pbl_transport_packet *packet);

void protocol_music_playpause();
void protocol_music_pause();
void protocol_music_play();
void protocol_music_next();
void protocol_music_prev();
void protocol_music_volup();
void protocol_music_voldown();
void protocol_music_message_send(uint8_t command_id);
