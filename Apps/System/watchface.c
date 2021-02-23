/* watchface.c
 * Routines for loading watchfaces based on
 * user preferences.
 * 
 * RebbleOS
 *
 */

#include "rebbleos.h"
#include "prefs.h"

/* Configure Logging */
#define MODULE_NAME "watchface"
#define MODULE_TYPE "WCH"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

static Uuid _watchface;

void rcore_watchface_prefs_save()
{
	(void) prefs_put(PREFS_KEY_WATCHFACE, &_watchface, sizeof(Uuid));
}

void rcore_watchface_prefs_load()
{
    if (uuid_null(&_watchface) || prefs_get(PREFS_KEY_WATCHFACE, &_watchface, sizeof(_watchface)) < 0)
    {
        LOG_DEBUG("No watchface preference set, using default.");
        App * default_watchface = appmanager_get_app_by_name("Simplicity");
        assert(default_watchface);

        memcpy(&_watchface, &default_watchface->uuid, sizeof(Uuid));
    }
    else
    {
        LOG_DEBUG("Loaded watchface preference.");
    }
    
    char buf[UUID_STRING_BUFFER_LENGTH];
    uuid_to_string(&_watchface, buf);

    LOG_INFO("Using watchface UUID: %s", buf);
}

void watchface_set_pref_by_uuid(Uuid *uuid)
{
    memcpy(&_watchface, uuid, sizeof(Uuid));

    char buf[UUID_STRING_BUFFER_LENGTH];
    uuid_to_string(&_watchface, buf);

    LOG_INFO("Setting watchface preference UUID: %s", buf);

    rcore_watchface_prefs_save();
}

void watchface_init()
{
    rcore_watchface_prefs_load();
}

void watchface_enter()
{
    appmanager_app_start_by_uuid(&_watchface);
}