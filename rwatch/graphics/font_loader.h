#pragma once
/* font_loader.h
 * routines for loading app and system fonts
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "fonts.h"

void fonts_resetcache();
GFont fonts_get_system_font(const char *key);
GFont fonts_load_custom_font(ResHandle handle, const struct file* file);
void fonts_unload_custom_font(GFont font);
GFont fonts_load_custom_font_proxy(ResHandle handle);

n_GGlyphInfo *fonts_glyphcache_get(GFont font, uint32_t codepoint);
void fonts_glyphcache_put(GFont font, uint32_t codepoint, n_GGlyphInfo *glyph);