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
#include "../types.h"
#include "../macros.h"
#include "../common.h"
#include "../context.h"

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                   Fonts                                      |
|                                                                              |
|                         Abstracts away font magicks.                         |
|                                                                              |
`-----------------------------------------------------------------------------*/

typedef struct n_GFontInfo {
    uint8_t version;
    uint8_t line_height;
    uint16_t glyph_amount;
    uint16_t wildcard_codepoint;
    // v2+
    uint8_t hash_table_size;
    uint8_t codepoint_bytes;
    // v3+
    uint8_t fontinfo_size;
    uint8_t features;
} __attribute__((__packed__)) n_GFontInfo;

typedef struct n_GFontHashTableEntry {
    uint8_t hash_value;
    uint8_t offset_table_size;
    uint16_t offset_table_offset;
} __attribute__((__packed__)) n_GFontHashTableEntry;

#define __FONT_INFO_V1_LENGTH 6
#define __FONT_INFO_V2_LENGTH 8

typedef enum {
    n_GFontFeature2ByteGlyphOffset = 0b1,
    n_GFontFeatureRLE4Encoding = 0b10,
} n_GFontFeatures;

typedef struct n_GGlyphInfo {
    uint8_t width;
    uint8_t height;
    int8_t left_offset;
    int8_t top_offset;
    int8_t advance;
    uint8_t data[];
} __attribute__((__packed__)) n_GGlyphInfo;

typedef n_GFontInfo * n_GFont;

uint8_t n_graphics_font_get_line_height(n_GFont font);

n_GGlyphInfo * n_graphics_font_get_glyph_info(n_GFont font, uint32_t charcode);

void n_graphics_font_draw_glyph(n_GContext * ctx, n_GGlyphInfo * glyph, n_GPoint p);
