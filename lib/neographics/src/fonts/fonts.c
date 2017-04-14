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

#include "fonts.h"

uint8_t n_graphics_font_get_line_height(n_GFontInfo * font) {
    return font->line_height;
}

n_GGlyphInfo * n_graphics_font_get_glyph_info(n_GFontInfo * font, uint32_t codepoint) {
    uint8_t * data;
    uint8_t hash_table_size = 255, codepoint_bytes = 4, features = 0;
    switch (font->version) {
        case 1:
            data = (uint8_t *) font + __FONT_INFO_V1_LENGTH;
            break;
        case 2:
            data = (uint8_t *) font + __FONT_INFO_V2_LENGTH;
            break;
        default:
            data = (uint8_t *) font + font->fontinfo_size;
    }
    switch (font->version) {
        // switch trickery! Default first is valid.
        default:
            features = font->features;
        case 2:
            hash_table_size = font->hash_table_size;
            codepoint_bytes = font->codepoint_bytes;
        case 1:
            break;
    }

    uint8_t offset_table_item_length = codepoint_bytes +
        (features & n_GFontFeature2ByteGlyphOffset ? 2 : 4);

    n_GFontHashTableEntry * hash_data =
        (n_GFontHashTableEntry *) (data +
            (codepoint % hash_table_size) * sizeof(n_GFontHashTableEntry));

    data += (hash_table_size * sizeof(n_GFontHashTableEntry));

    if (hash_data->hash_value != (codepoint % hash_table_size))
        // There was no hash table entry with the correct hash. Fall back to tofu.
        return (n_GGlyphInfo *) (data + offset_table_item_length * font->glyph_amount + 4);

    uint8_t * offset_entry = data + hash_data->offset_table_offset;

    uint16_t iters = 0; // theoretical possibility of 255 entries in an offset
                        // table mean that we can't use a uint8 for safety
    while ((codepoint_bytes == 2
               ? *((uint16_t *) offset_entry)
               : *((uint32_t *) offset_entry)) != codepoint &&
            iters < hash_data->offset_table_size) {
        offset_entry += offset_table_item_length;
        iters++;
    }

    if ((codepoint_bytes == 2
            ? *((uint16_t *) offset_entry)
            : *((uint32_t *) offset_entry)) != codepoint)
        // We couldn't find the correct entry. Fall back to tofu.
        return (n_GGlyphInfo *) (data + offset_table_item_length * font->glyph_amount + 4);

    data += offset_table_item_length * font->glyph_amount +
        (features & n_GFontFeature2ByteGlyphOffset
            ? *((uint16_t *) (offset_entry + codepoint_bytes))
            : *((uint32_t *) (offset_entry + codepoint_bytes)));

    n_GGlyphInfo * glyph = (n_GGlyphInfo *) data;

    return glyph;
}

void n_graphics_font_draw_glyph_bounded(n_GContext * ctx, n_GGlyphInfo * glyph,
    n_GPoint p, int16_t minx, int16_t maxx, int16_t miny, int16_t maxy) {
    p.x += glyph->left_offset;
    p.y += glyph->top_offset;
    for (uint8_t y = 0; y < glyph->height; y++)
        for (uint8_t x = 0; x < glyph->width; x++)
            if (glyph->data[(y*glyph->width+x)/8] & (1 << ((y*glyph->width+x) % 8)) &&
                    p.x + x >= minx && p.x + x < maxx &&
                    p.y + y >= miny && p.y + y < maxy)
                n_graphics_set_pixel(ctx, n_GPoint(p.x + x, p.y + y), ctx->text_color);
}

void n_graphics_font_draw_glyph(n_GContext * ctx, n_GGlyphInfo * glyph, n_GPoint p) {
    n_graphics_font_draw_glyph_bounded(ctx, glyph, p, 0, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}
