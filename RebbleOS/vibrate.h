#pragma once
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * All possible commands (with the exception of VIBRATE_CMD_MAX, which is only used to determine
 * the size of the array containing the default patterns)
 */
typedef enum
{
    VIBRATE_CMD_PLAY_PATTERN_1,
    VIBRATE_CMD_PALY_PATTERN_2,
    VIBRATE_CMD_PALY_PATTERN_3,
    VIBRATE_CMD_STOP,
    VIBRATE_CMD_MAX // add any other commands IDs _before_ this
} VibrateCmd_t;

/**
 * Struct which defines a pattern:
 *  buffer - contains the sequence of durations
 *  length - is the length of the buffer (a duration of 0 will be interpreted as silence)
 *  curBufferIndex - the index in the buffer of the current durration
 */
typedef struct
{
    const uint8_t length;
    const uint16_t * const buffer;
    uint8_t curBufferIndex;
} VibratePattern_t;

void vibrate_init(void);
void vibrate_command(VibrateCmd_t command);
void vibrate_play_pattern(VibratePattern_t *pattern);
void vibrate_stop(void);
