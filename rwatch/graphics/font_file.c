#include "fonts.h"
#include "font_loader.h"
#include "fs.h"

uint8_t n_graphics_font_get_line_height(struct file *font) {
    struct fd fd;
    n_GFontInfo info;
    
    /* XXX: cache this */
    fs_open(&fd, font);
    fs_read(&fd, &info, sizeof(info));
    return info.line_height;
}

n_GGlyphInfo * n_graphics_font_get_glyph_info(struct file *font, uint32_t codepoint) {
    struct fd fd;
    n_GFontInfo info;
    n_GGlyphInfo pglyph;
    
    n_GGlyphInfo *cglyph = fonts_glyphcache_get(font, codepoint);
    if (cglyph) {
        return cglyph;
    }
    
    /* XXX: cache this */
    fs_open(&fd, font);
    fs_read(&fd, &info, sizeof(info));

    uint8_t * data;
    uint8_t hash_table_size = 255, codepoint_bytes = 4, features = 0;
    switch (info.version) {
        case 1:
            fs_seek(&fd, __FONT_INFO_V1_LENGTH, FS_SEEK_SET);
            break;
        case 2:
            fs_seek(&fd, __FONT_INFO_V2_LENGTH, FS_SEEK_SET);
            break;
        default:
            fs_seek(&fd, info.fontinfo_size, FS_SEEK_SET);
    }
    switch (info.version) {
        // switch trickery! Default first is valid.
        default:
            features = info.features;
        case 2:
            hash_table_size = info.hash_table_size;
            codepoint_bytes = info.codepoint_bytes;
        case 1:
            break;
    }

    long loc = fs_seek(&fd, 0, FS_SEEK_CUR);

    uint8_t offset_table_item_length = codepoint_bytes +
        (features & n_GFontFeature2ByteGlyphOffset ? 2 : 4);

    /* Read the codepoint from the hash table ... */
    fs_seek(&fd, (codepoint % hash_table_size) * sizeof(n_GFontHashTableEntry), FS_SEEK_CUR);
    
    n_GFontHashTableEntry hash_data;
    fs_read(&fd, &hash_data, sizeof(hash_data));

    /* ... and seek past the rest of the hash table. */
    loc = fs_seek(&fd, loc + hash_table_size * sizeof(n_GFontHashTableEntry), FS_SEEK_SET);

    if (hash_data.hash_value != (codepoint % hash_table_size)) {
        // There was no hash table entry with the correct hash. Fall back to tofu.
        fs_seek(&fd, offset_table_item_length * info.glyph_amount + 4, FS_SEEK_CUR);
        goto readglyph;
    }
    
    /* It exists, so we find it in the offset table. */
    fs_seek(&fd, loc + hash_data.offset_table_offset, FS_SEEK_SET);
    uint8_t offset_entry[8]; /* 4 bytes max for codepoint, 4 bytes max for glyph offset */

    uint16_t iters = 0; // theoretical possibility of 255 entries in an offset
                        // table mean that we can't use a uint8 for safety
    do {
        fs_read(&fd, offset_entry, offset_table_item_length);
        iters++;
    } while ((codepoint_bytes == 2
                 ? *((uint16_t *) offset_entry)
                 : *((uint32_t *) offset_entry)) != codepoint &&
              iters <= hash_data.offset_table_size);

    if ((codepoint_bytes == 2
            ? *((uint16_t *) offset_entry)
            : *((uint32_t *) offset_entry)) != codepoint) {
        // We couldn't find the correct entry. Fall back to tofu.
        fs_seek(&fd, loc + offset_table_item_length * info.glyph_amount + 4, FS_SEEK_SET);
        goto readglyph;
    }

    fs_seek(&fd, loc + offset_table_item_length * info.glyph_amount +
        (features & n_GFontFeature2ByteGlyphOffset
            ? *((uint16_t *) (offset_entry + codepoint_bytes))
            : *((uint32_t *) (offset_entry + codepoint_bytes))),
        FS_SEEK_SET);

readglyph:
    /* How many bytes is a glyph? */
    fs_read(&fd, &pglyph, sizeof(pglyph));
    int gbits = pglyph.height * pglyph.width;
    int gbytes = (gbits + 7) / 8;
    
    n_GGlyphInfo *glyph = malloc(sizeof(pglyph) + gbytes);
    memcpy(glyph, &pglyph, sizeof(pglyph));
    fs_read(&fd, glyph + 1, gbytes);
    
    fonts_glyphcache_put(font, codepoint, glyph);

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
