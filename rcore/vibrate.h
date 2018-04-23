#pragma once
/* vibrate.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 *         Bogdan Olar  <>
 */

/**
 * All possible commands (with the exception of VIBRATE_CMD_MAX, which is only used to determine
 * the size of the array containing the default patterns)
 */
typedef enum
{
    VIBRATE_CMD_PLAY_PATTERN_1,
    VIBRATE_CMD_PLAY_PATTERN_2,
    VIBRATE_CMD_PLAY_PATTERN_3,
    VIBRATE_CMD_STOP,
    VIBRATE_CMD_MAX // add any other commands IDs _before_ this
} VibrateCmd_t;

/**
 * Each vibration pattern is composed of a series of pairs. Each pair contains a duration (during which the motor
 * spinns), and a spin frequency.
 */
typedef struct
{
    uint16_t frequency;
    uint16_t duration_ms;
} VibratePatternPair_t;

/**
 * Struct which defines a pattern:
 *  buffer - contains the sequence of durations, in milliseconds
 *  length - is the length of the buffer
 *  cur_buffer_index - the index in the buffer of the current durration
 */
typedef struct
{
    const uint8_t length;
    const VibratePatternPair_t * const buffer;
} VibratePattern_t;

void vibrate_init(void);
void vibrate_command(VibrateCmd_t command);
void vibrate_play_pattern(const VibratePattern_t *pattern);
void vibrate_stop(void);
