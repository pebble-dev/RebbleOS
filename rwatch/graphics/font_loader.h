#pragma once
/* font_loader.h
 * routines for loading app and system fonts
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

void fonts_resetcache();
GFont fonts_get_system_font(const char *key);
GFont *fonts_load_custom_font(ResHandle *handle, const struct file* file);
void fonts_unload_custom_font(GFont font);
GFont *fonts_load_custom_font_proxy(ResHandle *handle);
