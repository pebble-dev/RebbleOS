/*\
|*|
|*|   Neographics: a tiny graphics library.
|*|   Copyright (C) 2016 Johannes Neubrand <johannes_n@icloud.com>
|*|
|*|   This program is free software; you can redistribute it and/or
|*|   modify it under the terms of the GNU General Public License
|*|   as published by the Free Software Foundation; either version 2
|*|   of the License, or (at your option) any later version.
|*|
|*|   This program is distributed in the hope that it will be useful,
|*|   but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|   GNU General Public License for more details.
|*|
|*|   You should have received a copy of the GNU General Public License
|*|   along with this program; if not, write to the Free Software
|*|   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
|*|
\*/

#pragma once
#include <pebble.h>
#include "../path/path.h"
#include "../primitives/circle.h"

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                Draw Commands                                 |
|                                                                              |
|    Draw Commands were some of the most overlooked features of PebbleOS.      |
|    Neographics will continue to support them and even expand on them:        |
|    For example, you may have noticed that Draw Commands originally didn't    |
|    circles centered on subpixels, and that having multiple points in a       |
|    circle would lead to weird behavior in lieu of drawing multiple circles   |
|    with different centers.                                                   |
|                                                                              |
|    To facilitate these changes, neographics defines the Draw Command file    |
|    format version 2. Note that previously invalid files may now count as     |
|    valid. Neographics should be backwards-compatible with all valid files    |
|    from Draw Command version 1.                                              |
|                                                                              |
|    The `flags` byte (offset 1 in commands) now also lets users specify       |
|    specific fallback colors for b/w screens.                                 |
|                                                                              |
`-----------------------------------------------------------------------------*/

#define __N_DRAW_COMMAND_MAXIMUM_SUPPORTED_VERSION 2


typedef enum n_GDrawCommandType {
    n_GDrawCommandTypeInvalid = 0,
    n_GDrawCommandTypePath = 1,
    n_GDrawCommandTypeCircle = 2,
    n_GDrawCommandTypePrecisePath = 3,
    n_GDrawCommandTypePreciseCircle = 4,
} n_GDrawCommandType;

typedef enum n_GDrawCommandBWColor {
    n_GDrawCommandBWColorClear = 0,
    n_GDrawCommandBWColorBlack = 1,
    n_GDrawCommandBWColorGray  = 2,
    n_GDrawCommandBWColorWhite = 3,
} n_GDrawCommandBWColor;

typedef struct n_GDrawCommandFlags {
    bool hidden:1;
    bool use_bw_color:1;
    uint8_t bw_stroke:2; // 00: clear; 01: gray; 10: black; 11: white
    uint8_t bw_fill:2;
} n_GDrawCommandFlags;

typedef struct {
    n_GDrawCommandType type;
    n_GDrawCommandFlags flags;
    n_GColor stroke_color;
    uint8_t stroke_width;
    n_GColor fill_color;
    union {
        struct {
            bool path_open:1;
        } path_flags;
        uint16_t circle_radius;
    };
    uint16_t num_points;
    n_GPoint points[];
} __attribute((__packed__)) n_GDrawCommand;

typedef struct {
    uint16_t num_commands;
    n_GDrawCommand commands[];
} __attribute((__packed__)) n_GDrawCommandList;

typedef struct {
    uint8_t version;
    uint8_t reserved;
    n_GSize view_box;
    n_GDrawCommandList command_list[];
} __attribute((__packed__)) n_GDrawCommandImage;

typedef struct {
    uint16_t duration;
    n_GDrawCommandList command_list[];
} __attribute((__packed__)) n_GDrawCommandFrame;

typedef struct {
    uint8_t version;
    uint8_t reserved;
    n_GSize view_box;
    uint16_t play_count;
    uint16_t num_frames;
    n_GDrawCommandFrame frames[];
} __attribute((__packed__)) n_GDrawCommandSequence;

typedef bool (n_GDrawCommandListIteratorCb)(n_GDrawCommand * command, uint32_t index, void * context);

/* command getting/setting */

n_GDrawCommandType n_gdraw_command_get_type(n_GDrawCommand * command);

uint16_t n_gdraw_command_get_num_points(n_GDrawCommand * command);

n_GColor n_gdraw_command_get_stroke_color(n_GDrawCommand * command);
void     n_gdraw_command_set_stroke_color(n_GDrawCommand * command, n_GColor stroke_color);

uint8_t  n_gdraw_command_get_stroke_width(n_GDrawCommand * command);
void     n_gdraw_command_set_stroke_width(n_GDrawCommand * command, uint8_t stroke_width);

n_GColor n_gdraw_command_get_fill_color(n_GDrawCommand * command);
void     n_gdraw_command_set_fill_color(n_GDrawCommand * command, n_GColor fill_color);

n_GPoint n_gdraw_command_get_point(n_GDrawCommand * command, uint16_t index);
void     n_gdraw_command_set_point(n_GDrawCommand * command, uint16_t index, n_GPoint point);

uint16_t n_gdraw_command_get_radius(n_GDrawCommand * command);
void     n_gdraw_command_set_radius(n_GDrawCommand * command, uint16_t radius);

bool     n_gdraw_command_get_path_open(n_GDrawCommand * command);
void     n_gdraw_command_set_path_open(n_GDrawCommand * command, bool path_open);

bool     n_gdraw_command_get_hidden(n_GDrawCommand * command);
void     n_gdraw_command_set_hidden(n_GDrawCommand * command, bool hidden);

/* draw: defined for image / frame / sequence */

// NB these all take offsets. in the builtins, only image and frame drawing do.
void     n_gdraw_command_draw(n_GContext * ctx, n_GDrawCommand * command, n_GPoint offset);
void     n_gdraw_command_image_draw(n_GContext * ctx, n_GDrawCommandImage * image, n_GPoint offset);
void     n_gdraw_command_frame_draw(n_GContext * ctx, n_GDrawCommandSequence * sequence, n_GDrawCommandFrame * frame, n_GPoint offset);
void     n_gdraw_command_list_draw(n_GContext * ctx, n_GDrawCommandList * list, n_GPoint offset);

/* command list getters */

n_GDrawCommandList * n_gdraw_command_image_get_command_list(n_GDrawCommandImage * image);
n_GDrawCommandList * n_gdraw_command_frame_get_command_list(n_GDrawCommandFrame * frame);

uint32_t n_gdraw_command_list_get_num_commands(n_GDrawCommandList * list);
n_GDrawCommand * n_gdraw_command_list_get_command(n_GDrawCommandList * list, uint32_t index);

/* miscellaneous list-only */

void    n_gdraw_command_list_iterate(n_GDrawCommandList * list, n_GDrawCommandListIteratorCb cb, void * cb_context);

/* miscellaneous frame-only */

uint16_t n_gdraw_command_frame_get_duration(n_GDrawCommandFrame * frame);
void     n_gdraw_command_frame_set_duration(n_GDrawCommandFrame * frame, uint16_t duration);

/* miscellaneous sequence-only */

n_GDrawCommandFrame * n_gdraw_command_sequence_get_frame_by_elapsed(n_GDrawCommandSequence * sequence, uint32_t ms);
n_GDrawCommandFrame * n_gdraw_command_sequence_get_frame_by_index(n_GDrawCommandSequence * sequence, uint32_t index);
uint32_t n_gdraw_command_sequence_get_play_count(n_GDrawCommandSequence * sequence);
void     n_gdraw_command_sequence_set_play_count(n_GDrawCommandSequence * sequence, uint16_t play_count);

uint32_t n_gdraw_command_sequence_get_total_duration(n_GDrawCommandSequence * sequence);

uint16_t n_gdraw_command_sequence_get_num_frames(n_GDrawCommandSequence * sequence);

/* get / set bounds: defined for image and sequence */

n_GSize  n_gdraw_command_image_get_bounds_size(n_GDrawCommandImage * image);
void     n_gdraw_command_image_set_bounds_size(n_GDrawCommandImage * image, n_GSize size);

n_GSize  n_gdraw_command_sequence_get_bounds_size(n_GDrawCommandSequence * sequence);
void     n_gdraw_command_sequence_set_bounds_size(n_GDrawCommandSequence * sequence, n_GSize size);

/* create with resource / clone / destroy */

n_GDrawCommandImage * n_gdraw_command_image_create_with_resource(uint32_t resource_id);
n_GDrawCommandImage * n_gdraw_command_image_clone(n_GDrawCommandImage * image);
void n_gdraw_command_image_destroy(n_GDrawCommandImage * image);

n_GDrawCommandSequence * n_gdraw_command_sequence_create_with_resource(uint32_t resource_id);
n_GDrawCommandSequence * n_gdraw_command_sequence_clone(n_GDrawCommandSequence * image);
void n_gdraw_command_sequence_destroy(n_GDrawCommandSequence * sequence);

