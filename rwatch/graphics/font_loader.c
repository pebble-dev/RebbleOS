/* font_loader.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "librebble.h"
#include "platform_res.h"

// #define FONTS_DEBUG

/* totally hacky, but at least vaguely thread safe
 * We now have a cache size of 1 for each thread
 */

typedef struct GFontCache
{
    uint32_t resource_id;
    struct file font;
    struct GFontCache *next;
} GFontCache;

static void _fonts_glyphcache_purge(GFont font);
static void _fonts_glyphcache_reset();

uint16_t _fonts_get_resource_id_for_key(const char *key);
GFont fonts_get_system_font_by_resource_id(uint32_t resource_id);

static GFontCache *_app_font_cache = NULL;
static GFontCache *_ovl_font_cache = NULL;

void fonts_resetcache()
{
    struct GFontCache **cachep;
    
    KERN_LOG("font", APP_LOG_LEVEL_DEBUG, "Purging fonts");
    switch (appmanager_get_thread_type())
    {
    case AppThreadMainApp: _app_font_cache = NULL; break;
    case AppThreadOverlay: _ovl_font_cache = NULL; break;
    default:
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "Why you need fonts?");
        return;
    }
    
    _fonts_glyphcache_reset();
    /* We don't walk the chain of fonts deallocating them because we presume
     * that they got blown away along with the rest of the heap.  */
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
    GFontCache **cachep, *ent;
    AppThreadType thread_type = appmanager_get_thread_type();
    
    switch (thread_type)
    {
    case AppThreadMainApp: cachep = &_app_font_cache; break;
    case AppThreadOverlay: cachep = &_ovl_font_cache; break;
    default:
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "Why you need fonts?");
        return NULL;
    }
    
    ent = *cachep;
    while (ent) {
        if (ent->resource_id == resource_id)
            return &(ent->font);
        ent = ent->next;
    }
    
    ent = malloc(sizeof(*ent));
    if (!ent) {
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "font malloc failed");
        return NULL;
    }
    ent->resource_id = resource_id;
    resource_file(&ent->font, resource_get_handle_system(resource_id));
    ent->next = *cachep;
    *cachep = ent;

    return &ent->font;
}

/*
 * Load a custom font
 */
GFont fonts_load_custom_font(ResHandle handle, const struct file* ifile)
{
    GFont font = malloc(sizeof(struct file));
    if (!font) {
        KERN_LOG("font", APP_LOG_LEVEL_ERROR, "font malloc failed");
        return NULL;
    }
    resource_file_from_file_handle(font, ifile, handle);
    
    return font;
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
    _fonts_glyphcache_purge(font);
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

struct glyph_cache_ent
{
    uint32_t generation;
    GFont font;
    uint32_t codepoint;
    n_GGlyphInfo *glyph;
    struct glyph_cache_ent *next;
};

#define GLYPH_CACHE_MAXSIZ 128

static struct glyph_cache_ent *_app_glyph_cache = NULL;
static struct glyph_cache_ent *_ovl_glyph_cache = NULL;
static uint32_t _glyph_generation = 0; /* races on this are OK; just a hint */

static struct glyph_cache_ent **_thread_glyphcache() {
    switch (appmanager_get_thread_type())
    {
    case AppThreadMainApp: return &_app_glyph_cache;
    case AppThreadOverlay: return &_ovl_glyph_cache;
    default:
        assert(!"font glyph cache called from invalid thread");
        return NULL;
    }
}

n_GGlyphInfo *fonts_glyphcache_get(GFont font, uint32_t codepoint) {
    struct glyph_cache_ent *cache;
    
    for (cache = *_thread_glyphcache(); cache; cache = cache->next)
        if (cache->font == font && cache->codepoint == codepoint) {
            cache->generation = _glyph_generation++;
            return cache->glyph;
        }
    
    return NULL;
}

void fonts_glyphcache_put(GFont font, uint32_t codepoint, n_GGlyphInfo *glyph) {
    struct glyph_cache_ent *cache;
    struct glyph_cache_ent *oldest = NULL;
    uint32_t oldest_gen = 0;
    int oldestidx = 0;
    int cachelen = 0;
    
    /* We assume it's not already in the cache, so we simultaneously compute
     * how large the cache is and find the least recently used object, in
     * case we need to kick something out.
     */
    for (cache = *_thread_glyphcache(); cache; cache = cache->next) {
        assert(cache->font != font || cache->codepoint != codepoint);
        if (cache->font == NULL) {
            /* already purged -- overwrite this entry */
            oldest = cache;
            break;
        }
        if ((_glyph_generation - cache->generation) >= oldest_gen) {
            oldest = cache;
            oldest_gen = _glyph_generation - cache->generation;
            oldestidx = cachelen;
        }
        cachelen++;
    }
    
    if ((cachelen == GLYPH_CACHE_MAXSIZ) || (oldest && !oldest->glyph)) {
        /* Someone's getting evicted. */
#ifdef FONTS_DEBUG
        printf("evicting font %p codepoint %x idx %d clen %d\n", font, codepoint, oldestidx, cachelen);
#endif
        oldest->font = font;
        oldest->codepoint = codepoint;
        if (oldest->glyph);
        free(oldest->glyph);
        oldest->glyph = glyph;
        oldest->generation = _glyph_generation++;
    } else {
#ifdef FONTS_DEBUG
        printf("allocating new %p %lx, clen %d, ttype %d\n", font, codepoint, cachelen,appmanager_get_thread_type());
#endif
        /* Allocate something new. */
        cache = malloc(sizeof(*oldest));
        if (cache)
            cache->next = *_thread_glyphcache();
        else { /* out of memory? */
            cache = *_thread_glyphcache();
            free(cache->glyph);
        }
        if (!cache) /* seriously out of memory? */ {
            KERN_LOG("font", APP_LOG_LEVEL_ERROR, "cache malloc failed");
            return; /* leaks the glyph!! */
        }
        cache->generation = _glyph_generation++;
        cache->font = font;
        cache->codepoint = codepoint;
        cache->glyph = glyph;
        *_thread_glyphcache() = cache;
    }
}

static void _fonts_glyphcache_purge(GFont font) {
    struct glyph_cache_ent *cache;
    
    for (cache = *_thread_glyphcache(); cache; cache = cache->next)
        if (cache->font == font) {
            cache->font = NULL;
            free(cache->glyph);
            cache->glyph = NULL;
        }
}

static void _fonts_glyphcache_reset() {
    *_thread_glyphcache() = NULL;
}
