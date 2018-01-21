/* appmanager_app.c
 * Routines for dealing with scanning and reading of
 * internal apps and apps on flash
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"
#include "widgettest.h"
#include "notification.h"
#include "test_defs.h"
#include "node_list.h"
#include "rdb.h"
#include "blobdb.h"
#include "music.h"

static App *_appmanager_create_app(char *name, Uuid *uuid, uint32_t app_id, uint8_t type, void *entry_point, bool is_internal,
                                   const struct file *app_file, const struct file *resource_file);
static void _appmanager_flash_load_app_manifest();
static void _appmanager_add_to_manifest(App *app);
static void _appmanager_flash_load_app_manifest_n(void);

/* simple doesn't have an include, so cheekily forward declare here */
void simple_main(void);
void nivz_main(void);

/* note that these flags are inverted */
#define APPDB_DBFLAGS_WRITTEN 1
#define APPDB_DBFLAGS_OVERWRITING 2
#define APPDB_DBFLAGS_DEAD 4

#define APPDB_FLAGS_IS_WATCHFACE 1

#define APPDB_IS_EOF(ent) (((ent).last_modified == 0xFFFFFFFF) && ((ent).hash == 0xFF) && ((ent).dbflags == 0x3F) && ((ent).key_length == 0x7F) && ((ent).value_length == 0x7FF))

struct appdb
{
    /* below is common to all settings DB entries */
    uint32_t last_modified;  //0x58F6AExx
    uint8_t hash;
    uint8_t dbflags:6;
    uint32_t key_length:7;
    uint32_t value_length:11;

    uint32_t application_id;
    
    Uuid app_uuid;  // 16 bytes
    uint32_t flags; /* pebble_process_info.h, PebbleProcessInfoFlags in the SDK */
    uint32_t icon;
    uint8_t app_version_major, app_version_minor;
    uint8_t sdk_version_major, sdk_version_minor;
    uint8_t app_face_bg_color, app_face_template_id;
    uint8_t app_name[32];
    uint8_t unk_arr_company[32];  // always blank
    uint8_t unk_arr[32]; // always blank
} __attribute__((__packed__));

typedef struct appdb_n
{
//     uint32_t application_id; this is the rdb key
    Uuid app_uuid;  // 16 bytes
    uint32_t flags; /* pebble_process_info.h, PebbleProcessInfoFlags in the SDK */
    uint32_t icon;
    uint8_t app_version_major, app_version_minor;
    uint8_t sdk_version_major, sdk_version_minor;
    uint8_t app_face_bg_color, app_face_template_id;
    uint8_t app_name[32];
    uint8_t unk_arr_company[32];  // always blank
    uint8_t unk_arr[32]; // always blank
} __attribute__((__packed__)) appdb_n ;

static list_head _app_manifest_head = LIST_HEAD(_app_manifest_head);


void appmanager_app_loader_init_n()
{
    _appmanager_flash_load_app_manifest_n();
}

/*
 * Load any pre-existing apps into the manifest, search for any new ones and then start up
 */

void appmanager_app_loader_init()
{
    struct file empty = { 0, 0, 0 }; /* TODO: make files optional in `App` to avoid this */
    
    /* XXX: We need to completely clear the app manifest each time we reload the rdb, really. */
    
    /* add the baked in apps */
    _appmanager_add_to_manifest(_appmanager_create_app("System", 
                                                       NULL, 9991, 
                                                       AppTypeSystem, systemapp_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Simple", 
                                                       NULL, 9992, 
                                                       AppTypeWatchface, simple_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("NiVZ", 
                                                       NULL, 9993, 
                                                       AppTypeWatchface, nivz_main, true, &empty, &empty));
//     _appmanager_add_to_manifest(_appmanager_create_app("Settings", AppTypeSystem, test_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Notification", 
                                                       NULL, 9994, 
                                                       AppTypeSystem, notif_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("TestApp", 
                                                       NULL, 9995, 
                                                       AppTypeSystem, testapp_main, true, &empty, &empty));
    _appmanager_add_to_manifest(_appmanager_create_app("Music", APP_TYPE_SYSTEM, music_main, true, &empty, &empty));
    
    /* now load the ones on flash */
    _appmanager_flash_load_app_manifest();
    
    appmanager_app_loader_init_n();
}


/*
 * 
 * Generate an entry in the application manifest for each found app.
 * 
 */
static App *_appmanager_create_app(char *name, Uuid *uuid, uint32_t app_id, uint8_t type, void *entry_point, bool is_internal,
                                   const struct file *app_file, const struct file *resource_file)
{
    App *app = calloc(1, sizeof(App));
    if (app == NULL)
        return NULL;
        
    app->name = calloc(1, strlen(name) + 1);
    
    if (app->name == NULL)
        return NULL;
    
    strcpy(app->name, name);
    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Creating App %s", app->name);
    app->main = (void*)entry_point;
    app->type = type;
    app->header = NULL;
    
    appmanager_app_set_flag(app, AppFilePresent, app_file == NULL ? false : true);
    appmanager_app_set_flag(app, ResourceFilePresent, resource_file == NULL ? false : true);
    
    if (app_file)
        memcpy(&app->app_file, app_file, sizeof(struct file));
    if (resource_file)
        memcpy(&app->resource_file, resource_file, sizeof(struct file));
    
    appmanager_app_set_flag(app, ExecuteFromInternalFlash, is_internal);
    app->id = app_id;
    
    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "Created App %s", app->name);

    if (!uuid)
    {
        uint8_t uuidb[16];
        memset(uuidb, 0xFF, sizeof(uuidb));
        memcpy(uuidb, &app_id, 4);
        memcpy(&app->uuid, uuidb, sizeof(Uuid));
    }
    else
    {
        memcpy(&app->uuid, uuid, sizeof(Uuid));
    }
    
    return app;
}

static void _appmanager_flash_load_app_manifest_n(void)
{
    list_head head;
    list_init_head(&head);
    
    struct rdb_database *db = rdb_open(RDB_ID_APP);
    struct rdb_iter it;
    if (rdb_iter_start(db, &it) == 0) {
        rdb_close(db);
        return;
    }
    
    int zero = 0;
    struct rdb_selector selectors[] = {
        { offsetof(appdb_n, app_name), FIELD_SIZEOF(appdb_n, app_name), RDB_OP_RESULT },
        { offsetof(appdb_n, app_uuid), FIELD_SIZEOF(appdb_n, app_uuid), RDB_OP_RESULT },
        { offsetof(appdb_n, flags), FIELD_SIZEOF(appdb_n, flags), RDB_OP_RESULT },
        { }
    };
    int count = rdb_select(&it, &head, selectors);
        
    struct rdb_select_result *res;
    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "found %d apps", count);
    rdb_select_result_foreach(res, &head) {
        uint32_t appid = *(uint32_t *)res->key;
        
        /* does it have a file? */
        struct file appfile, resfile;
        int hasapp, hasres;
        char fname[14];
        snprintf(fname, 14, "@%08lx/app", appid);
        hasapp = fs_find_file(&appfile, fname) >= 0;
        snprintf(fname, 14, "@%08lx/res", appid);
        hasres = fs_find_file(&resfile, fname) >= 0;

        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "FOUND App %d (%s) with key %08x (app %s, res %s)", count, (char *)res->result[0], appid,
            hasapp ? "present" : "missing",
            hasres ? "present" : "missing");
        
        /* main gets set later */
        App *app = _appmanager_create_app((char *)res->result[0],
                                                           (Uuid *)res->result[1],
                                                           *(uint32_t *)res->key,
                                                           ((*(uint32_t *)res->result[2]) & APPDB_FLAGS_IS_WATCHFACE) ? AppTypeWatchface : AppTypeApp,
                                                           NULL,
                                                           false,
                                                           hasapp ? &appfile : NULL,
                                                           hasres ? &resfile : NULL);
        
        _appmanager_add_to_manifest(app);
    }
    rdb_close(db);
    rdb_select_free_all(&head);
}

/*
 * Load the list of apps and faces from flash
 * The app manifest is a list of all known applications we found in flash
 * We load all entries from `appdb` file.
 * TODO: appdb seems to have duplicates (in my case), maybe we should use `pmap`, but it was missing some entries for me
 */
static void _appmanager_flash_load_app_manifest(void)
{
    struct file file;

    if (fs_find_file(&file, "appdb") < 0)
    {
        KERN_LOG("app", APP_LOG_LEVEL_ERROR, "APPDB file not found");
        return;
    }

    char buffer[14];
    struct appdb appdb;
    struct fd fd;
    struct file app_file;
    struct file res_file;
    struct fd app_fd;
    ApplicationHeader header;

    fs_open(&fd, &file);

    /* skipping 8 bytes for appdb file header */
    fs_seek(&fd, 8, FS_SEEK_SET);
    for (int i = 0; i < file.size / sizeof(struct appdb); ++i) {
        if (fs_read(&fd, &appdb, sizeof(appdb)) != sizeof(appdb))
            break;

        if (APPDB_IS_EOF(appdb))
            break;

        if (appdb.dbflags & APPDB_DBFLAGS_WRITTEN) {
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file that is not written before eof, at index %d", i);
            continue;
        }
        
        if ((appdb.dbflags & APPDB_DBFLAGS_DEAD) == 0)
            continue;
        
        if ((appdb.dbflags & APPDB_DBFLAGS_OVERWRITING) == 0)
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file %08x is mid-overwrite; I feel nervous", appdb.application_id);

        if (appdb.application_id == 0xFFFFFFFFu) {
            KERN_LOG("app", APP_LOG_LEVEL_WARNING, "appdb: file is written, but has no contents?");
            break;
        }
        
        snprintf(buffer, 14, "@%08lx/app", appdb.application_id);
        if (fs_find_file(&app_file, buffer) < 0)
            continue;

        snprintf(buffer, 14, "@%08lx/res", appdb.application_id);
        if (fs_find_file(&res_file, buffer) < 0)
            continue;

        fs_open(&app_fd, &app_file);

        if (fs_read(&app_fd, &header, sizeof(ApplicationHeader)) != sizeof(ApplicationHeader))
            break;
       
        /* sanity check the hell out of this to make sure it's a real app */
        if (strncmp(header.header, "PBLAPP", 6))
        {
            KERN_LOG("app", APP_LOG_LEVEL_ERROR, "No PBLAPP header!");
            continue;
        }
        
        
        /* it's real... so far. Lets crc check to make sure
            * TODO
            * crc32....(header.header)
            */
        KERN_LOG("app", APP_LOG_LEVEL_INFO, "appdb: app \"%s\" found, id %08x, flags %02x, kl %d, vl %d, flags %08x, icon %08x", header.name, appdb.application_id, appdb.dbflags, appdb.key_length, appdb.value_length, appdb.flags, appdb.icon);

        /* main gets set later */
        _appmanager_add_to_manifest(_appmanager_create_app(header.name, 
                                                           (Uuid *)&appdb.app_uuid,
                                                           appdb.application_id,
                                                           (appdb.flags & APPDB_FLAGS_IS_WATCHFACE) ? AppTypeWatchface : AppTypeApp,
                                                           NULL,
                                                           false,
                                                           &app_file,
                                                           &res_file));
    }
}

/* 
 * App manifest is a linked list. Just slot it in 
 */
static void _appmanager_add_to_manifest(App *app)
{  
    list_init_node(&app->node);
    if (list_get_head(&_app_manifest_head) == NULL)
    {
        list_insert_head(&_app_manifest_head, &app->node);
        return;
    }
    
    list_insert_tail(&_app_manifest_head,&app->node);
}

/*
 * Get the top level node for the app manifest
 */
list_head *app_manager_get_apps_head()
{
    return &_app_manifest_head;
}

/*
 * Get an application by name. NULL if invalid
 */
App *appmanager_get_app_by_name(char *app_name)
{
    App * app;
    list_foreach(app, &_app_manifest_head, App, node)
    {
        if (!strncmp(app->name, (char *)app_name, strlen(app->name)))
            return app;
    }
    KERN_LOG("app", APP_LOG_LEVEL_ERROR, "NO App Found %s", app_name);
    return NULL;
}


/*
 * Get an application by name. NULL if invalid
 */
App *appmanager_get_app_by_id(uint32_t id)
{
    App * app;
    list_foreach(app, &_app_manifest_head, App, node)
    {
        if (app->id == id)
            return app;
    }

    return NULL;
}

App *appmanager_get_app_by_uuid(Uuid *uuid)
{
    App * app;
    list_foreach(app, &_app_manifest_head, App, node)
    {
        if (uuid_equal(&app->uuid, uuid))
            return app;
    }

    return NULL;
}

uint32_t appmanager_get_next_appid(void)
{
    App * app;
    uint32_t max = 0;
    list_foreach(app, &_app_manifest_head, App, node)
    {
        if (app->id > max && app->id < 9000)
            max = app->id;
    }
    KERN_LOG("app", APP_LOG_LEVEL_INFO, "Max app id found %d", max + 1);
    return max + 1;
}
