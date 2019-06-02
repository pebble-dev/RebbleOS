/* font_loader.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "librebble.h"
#include "platform_res.h"




/* totally hacky, but at least vaguely thread safe
 * We now have a cache size of 1 for each thread
 */

typedef struct GFontCache
{
    uint32_t resource_id;
    GFont font;
} GFontCache;

uint16_t _fonts_get_resource_id_for_key(const char *key);
GFont fonts_get_system_font_by_resource_id(uint32_t resource_id);

static GFontCache _app_font_cache;
static GFontCache _ovl_font_cache;

void fonts_resetcache()
{
    AppThreadType thread_type = appmanager_get_thread_type();
    
    KERN_LOG("font", APP_LOG_LEVEL_DEBUG, "Purging fonts");
    /* This is pretty terrible. We assume that the app is removing the memory 
     * for the font before we kill the cache entry */
    if (thread_type == AppThreadMainApp)
    {
        /* reset it to available. */
        _app_font_cache.resource_id = 0;
        app_free(_app_font_cache.font);
        _app_font_cache.font = NULL;
    }
    else if (thread_type == AppThreadOverlay)
    {
        /* reset it to available. */
        _ovl_font_cache.resource_id = 0;
        app_free(_ovl_font_cache.font);
        _ovl_font_cache.font = NULL;
    }
    else
    {
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "Why you need fonts?");
    }
}

// get a system font and then cache it. Ugh.
// TODO make this not suck (RAM)
GFont fonts_get_system_font(const char *font_key)
{
    uint16_t res_id = _fonts_get_resource_id_for_key(font_key);
    
    return fonts_get_system_font_by_resource_id(res_id);
}

/*
 * Load a system font from the resource table
 * Will save into a cheesey cache so it isn't loaded over and over.
 */
GFont fonts_get_system_font_by_resource_id(uint32_t resource_id)
{
    GFontCache *cache_item;
    AppThreadType thread_type = appmanager_get_thread_type();  
    
    if (thread_type == AppThreadMainApp)
    {
        cache_item = &_app_font_cache;
    }
    else if (thread_type == AppThreadOverlay)
    {
        cache_item = &_ovl_font_cache;
    }
    else
    {
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "Why you need fonts?");
    }

    if (cache_item->resource_id == resource_id)
    {
        return cache_item->font;
    }
    
    /* not cached, load */   
    if (cache_item->resource_id > 0 && cache_item->font)
    {
        app_free(cache_item->font);
    }
    
    uint8_t *buffer = resource_fully_load_id_system(resource_id);
    
    cache_item->font = (GFont)buffer;
    cache_item->resource_id = resource_id;

    return cache_item->font;
}

/*
 * Load a custom font
 */
GFont fonts_load_custom_font(ResHandle handle, const struct file* file)
{
    uint8_t *buffer = resource_fully_load_resource(handle, file, NULL);
    
    return (GFont)buffer;
}

GFont fonts_load_custom_font_proxy(ResHandle handle)
{
    App *app = appmanager_get_current_app();
    return (GFont)fonts_load_custom_font(handle, &app->resource_file);
}

/*
 * Unload a custom font
 */
void fonts_unload_custom_font(GFont font)
{
    app_free(font);
}

#define EQ_FONT(font) (strncmp(key, "RESOURCE_ID_" #font, strlen(key)) == 0) return RESOURCE_ID_ ## font;

/*
 * Load a font by a string key
 */
uint16_t _fonts_get_resource_id_for_key(const char *key)
{   
    // this seems kinda.... messy and bad. Why a char * Pebble? why pass strings around?
    // I got my answer from Heiko:
    /*
     @Heiko 
     That API has been around forever. Strings are an easy way to maintain an ABI contract between app and firmware compiled at different times. Was helpful as different SDK versions and models introduced various new fonts over time. It also allows for "secret fonts" that were not (yet) published. While one could accomplish the same with enums that have gaps and vary over time we already had those names in the firmware. And of course we had to maintain backwards compatibility when one of the fonts was renamed... again old API :wink:
      
     */
    // so still seems like a bad choice, but backward compat.
         if EQ_FONT(AGENCY_FB_60_THIN_NUMBERS_AM_PM)
    else if EQ_FONT(AGENCY_FB_60_NUMBERS_AM_PM)
    else if EQ_FONT(AGENCY_FB_36_NUMBERS_AM_PM)
    else if EQ_FONT(GOTHIC_09)
    else if EQ_FONT(GOTHIC_14)
    else if EQ_FONT(GOTHIC_14_BOLD)
    else if EQ_FONT(GOTHIC_18)
    else if EQ_FONT(GOTHIC_18_BOLD)
    else if EQ_FONT(GOTHIC_24)
    else if EQ_FONT(GOTHIC_24_BOLD)
    else if EQ_FONT(GOTHIC_28)
    else if EQ_FONT(GOTHIC_28_BOLD)
    else if EQ_FONT(GOTHIC_36)
    else if EQ_FONT(BITHAM_18_LIGHT_SUBSET)
    else if EQ_FONT(BITHAM_34_LIGHT_SUBSET)
    else if EQ_FONT(BITHAM_30_BLACK)
    else if EQ_FONT(BITHAM_42_BOLD)
    else if EQ_FONT(BITHAM_42_LIGHT)
    else if EQ_FONT(BITHAM_34_MEDIUM_NUMBERS)
    else if EQ_FONT(BITHAM_42_MEDIUM_NUMBERS)
    else if EQ_FONT(ROBOTO_CONDENSED_21)
    else if EQ_FONT(ROBOTO_BOLD_SUBSET_49)
    else if EQ_FONT(DROID_SERIF_28_BOLD)
    else if EQ_FONT(LECO_20_BOLD_NUMBERS)
    else if EQ_FONT(LECO_26_BOLD_NUMBERS_AM_PM)
    else if EQ_FONT(LECO_32_BOLD_NUMBERS)
    else if EQ_FONT(LECO_36_BOLD_NUMBERS)
    else if EQ_FONT(LECO_38_BOLD_NUMBERS)
    else if EQ_FONT(LECO_28_LIGHT_NUMBERS)
    else if EQ_FONT(LECO_42_NUMBERS)
    else if EQ_FONT(FONT_FALLBACK)
                                                                                                                                
    return RESOURCE_ID_FONT_FALLBACK;
}
