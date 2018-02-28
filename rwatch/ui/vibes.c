/* vibes.c
 * Provides vibration services implementation.
 * RebbleOS
 *
 * Author: Elliot Hawkins <elliotshawkins@gmail.com>
 */

#include <stdbool.h>
#include "vibes.h"
#include "vibrate.h"
#include "rebbleos.h"

static VibratePattern_t short_pattern = {
    .length = 1,
    .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 250}},
    .cur_buffer_index = 0
};

static VibratePattern_t long_pattern = {
    .length = 1,
    .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 500}},
    .cur_buffer_index = 0
};

static VibratePattern_t double_pattern = {
    .length = 3,
    .buffer = (const VibratePatternPair_t [])  {{.frequency = 255, .duration_ms = 100},
                                                {.frequency = 0, .duration_ms = 100},
                                                {.frequency = 255, .duration_ms = 100}},
    .cur_buffer_index = 0
};

static VibratePattern_t custom_pattern;
static VibratePatternPair_t custom_pattern_pairs[20];

void vibes_cancel(){
    vibrate_stop();
}

void vibes_short_pulse() {
  vibrate_play_pattern(&short_pattern);
}

void vibes_long_pulse() {
  vibrate_play_pattern(&long_pattern);
}

void vibes_double_pulse() {
  vibrate_play_pattern(&double_pattern);
}

void vibes_enqueue_custom_pattern(VibePattern pattern) {
    //Convert Pebble API vibrate patterns into an array of Rebble API vibrate pattern pairs
    for (int index = 0; index < pattern.num_segments; index++){
        //In Pebble API, even indicies are motor on, odd are motor off.
        custom_pattern_pairs[index].frequency = ((index % 2 == 0) ? 255 : 0);
        custom_pattern_pairs[index].duration_ms = pattern.durations[index];
    }

    //Rebble VibratePattern_t has read only members, but we need to keep in memory so vibrate thread can properly
    //play it. Create locally then copy to global variable
    VibratePattern_t native_pattern = {
        .length = pattern.num_segments,
        .buffer = custom_pattern_pairs,
        .cur_buffer_index = 0
    };
    memcpy(&custom_pattern, &native_pattern, sizeof(VibratePattern_t));

    vibrate_play_pattern(&custom_pattern);
}
