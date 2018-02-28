#pragma once
/* vibes.h
 * Provides vibration services declaration.
 * RebbleOS
 *
 * Author: Elliot Hawkins <elliotshawkins@gmail.com>
 */

#include <stdint.h>

typedef struct VibePattern {
    const uint32_t *durations; //Segment durrations in milliseconds
    uint32_t num_segments;
} VibePattern;

void vibes_cancel();
void vibes_short_pulse();
void vibes_long_pulse();
void vibes_double_pulse();
void vibes_enqueue_custom_pattern(VibePattern pattern);
