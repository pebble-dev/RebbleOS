/* overlay_test.c
 * Routines for testing the overlay window
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

#include "rebbleos.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"
#include "overlay_manager.h"
#include "notification_layer.h"
#include "platform_res.h"

static Window *_main_window;
static Layer *_test_layer;
static Layer *_overlay_layer;
static Layer *_overlay_layer2;
static NotificationLayer *_notif_layer;
static OverlayWindow *_overlay_window;
static OverlayWindow *_overlay_window2;
static OverlayWindow *_overlay_window3;
static void _color_test_layer_update_proc(Layer *layer, GContext *ctx);
static void _creation_callback(OverlayWindow *overlay_window, Window *window);
static void _creation_callback2(OverlayWindow *overlay_window, Window *window);
void _ovl_tick(struct tm *tick_time, TimeUnits tick_units);
static void _test_layer_update_proc(Layer *layer, GContext *nGContext);
static void _notif_init(OverlayWindow *overlay_window, Window *window);
static void _notif_test_window_load(Window *window);
static void _notif_test_window_unload(Window *window);
bool _notif_deinit(void);

static int _total_elapsed;
static GColor _genned_color;
static bool _test_complete = false;
static uint8_t _test_stage = 0;
static int _sub_stage = 0;
    
bool _test_overlay_creation_test(void *callback)
{   
    switch(_sub_stage) {
        case 0:
            overlay_window_create(callback);
            _sub_stage = 1;
            return false;
        case 1:
            test_assert_point_is_not_null(_overlay_window);
            _sub_stage = 0;
            return true;
    }
    return false; /* uh? */
}

bool _test_overlay_creation()
{
    return _test_overlay_creation_test(&_creation_callback);
}

bool _test_overlay_creation2()
{
    return _test_overlay_creation_test(&_creation_callback2);
}

bool _test_overlay_draw()
{
    /* test the overlay window for it's colour */
    test_assert_point_is_color(GPoint(9, 9),  _genned_color);
    test_assert_point_is_color(GPoint(10, 10), GColorRed);
    test_assert_point_is_color(GPoint(59, 59), GColorRed);
    test_assert_point_is_color(GPoint(60, 60), _genned_color);
    
    return true;
}

bool _test_overlay_destroy_test(Layer **ol, OverlayWindow **ow, GPoint point, GColor color)
{
     switch(_sub_stage) {
        case 0:
            if (*ol)
                layer_destroy(*ol);
            *ol = NULL;
            overlay_window_destroy(*ow);
            *ow = NULL;
            _sub_stage = 1;
            return false;
        case 1:
            /* should be deleted, and also it should not be painting */
            test_assert_point_is_null(*ow);
            test_assert_point_is_color(point, color);
            _sub_stage = 0;
            return true;
    }
    return false; /* uh? */
}

bool _test_overlay_destroy()
{
    return _test_overlay_destroy_test(&_overlay_layer, &_overlay_window, GPoint(10, 10), _genned_color);
}

bool _test_overlay_destroy2()
{
    return _test_overlay_destroy_test(&_overlay_layer2, &_overlay_window2, GPoint(10, 10), _genned_color);
}

bool _test_overlay_notif()
{
    overlay_window_create(_notif_init);
    return true;
}


bool _test_notif_destroy_test(NotificationLayer **ol, OverlayWindow **ow, GPoint point, GColor color)
{
     switch(_sub_stage) {
        case 0:
            overlay_window_destroy(*ow);
            *ow = NULL;
            _sub_stage = 1;
            return false;
        case 1:
            /* should be deleted, and also it should not be painting */
            test_assert_point_is_null(*ow);
            test_assert_point_is_color(point, color);
            _sub_stage = 0;
            return true;
    }
    return false; /* uh? */
}


bool _test_overlay_notif_destroy()
{
    static uint8_t ss = 0;
     switch(ss) {
        case 0:
            test_assert_point_is_color(GPoint(10,10), GColorFromRGB(85, 0, 170));
//             _notif_deinit();
            ss = 1;
            return false;
        case 1:
            
            if (_test_notif_destroy_test(&_notif_layer, &_overlay_window3, GPoint(10, 60), _genned_color))
            {
                ss = 0;
                return true;
            }
            return false;     
     }
     
     return false;
}

void _test_progress(bool is_from_overlay)
{
    if (_test_complete)
        return;
    _total_elapsed++;
    if (_total_elapsed < 2)
        return;
    if(_test_stage == 0)
    {
        if (!_test_overlay_creation())
            return;
        _test_stage++;
    }
    if(_test_stage == 1 && _total_elapsed > 3)
    {
        /* Check the background colour works */
        test_assert_point_is_color(GPoint(0, 0), _genned_color);
        _test_stage++;
    }
    if(_test_stage == 2)
    {
        _test_overlay_draw();        
        _test_stage++;
    }
    if(_test_stage == 3)
    {
        APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "===== TEST 3");
        /* two windows */
        if (!_test_overlay_creation2())
            return;
        _test_stage++;
    }
    if(_test_stage == 4&& _total_elapsed > 5)
    {
        APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "===== TEST 4");
        if (!_test_overlay_destroy())
            return;
        _test_stage++;
    }
    if(_test_stage == 5 && _total_elapsed > 7)
    {
        APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "===== TEST 5");
        if (!_test_overlay_destroy2())
            return;
        _test_stage++;
    }
    if(_test_stage == 6 && _total_elapsed > 9)
    {
        APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "===== TEST 6");
        if (!_test_overlay_notif())
        {
            return;
        }
        _test_stage++;
    }
    if(_test_stage == 7 && _total_elapsed > 12)
    {
        APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "===== TEST 7");
        if (!_test_overlay_notif_destroy())
            return;
        _test_complete = true;
    }
    
    if (_test_complete && !is_from_overlay)
    {
        _total_elapsed = 0;
        test_complete(test_get_success());
    }
}

bool overlay_test_init(Window *window)
{
    APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "Init: Overlay Test");
    _main_window = window;
    _test_complete = false;
    /* tests will have a partial overlay */
    window->background_color = GColorClear;
    
    /* Setup a layer we are going to draw like an app would */
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    _test_layer = layer_create(bounds);
    layer_add_child(window_layer, _test_layer);
    
    tick_timer_service_subscribe(SECOND_UNIT, _ovl_tick);
    layer_set_update_proc(_test_layer, _test_layer_update_proc);
    layer_mark_dirty(_test_layer);

    
    return true;
}

bool overlay_test_exec(void)
{
    APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "Exec: Overlay Test");
        
    _test_progress(false);
    return true;
}

bool overlay_test_deinit(void)
{
    APP_LOG("ovltst", APP_LOG_LEVEL_ERROR, "De-Init: Overlay Test");
    layer_remove_from_parent(_test_layer);
    layer_remove_from_parent(_overlay_layer);
    
    if (_test_layer)
        layer_destroy(_test_layer);

    _test_layer = NULL;

    return true;
}

/* Test app layer */

static GColor _gen_color()
{
    uint8_t s_color_channels[3];
    for (int i = 0; i < 3; i++)
    {
        s_color_channels[i] = rand() % 256;
    }
    GColor c = GColorFromRGB(s_color_channels[0], s_color_channels[1], s_color_channels[2]);
    /* If it's the same colour as our ovrlay, roll the dice again */
    if (c.argb == GColorRed.argb || c.argb == GColorBlack.argb)
        return _gen_color();
    return c;
}

static void _test_layer_update_proc(Layer *layer, GContext *nGContext)
{
    GRect full_bounds = layer_get_bounds(layer);
    _genned_color = _gen_color();
    graphics_context_set_fill_color(nGContext, _genned_color);    
    graphics_fill_rect(nGContext, full_bounds, 0, GCornerNone);
}


void _ovl_tick(struct tm *tick_time, TimeUnits tick_units)
{
    /* test the buffer before we paint all over it */
    _test_progress(false);
    layer_mark_dirty(_test_layer);
}

/* */

/* Overlay window */

static void _overlayer_update(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_rect(ctx, GRect(10, 10, 50, 50), 0, GCornerNone);
}

static void _creation_callback(OverlayWindow *overlay_window, Window *window)
{
    APP_LOG("overlay", APP_LOG_LEVEL_INFO, "Overlay Create CB");
    _overlay_window = overlay_window;
    
    window->background_color = GColorClear;
    
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    _overlay_layer = layer_create(bounds);
    layer_add_child(window_layer, _overlay_layer);
    
    layer_set_update_proc(_overlay_layer, _overlayer_update);
    
    overlay_window_stack_push(overlay_window, true);
    layer_mark_dirty(_overlay_layer);
}

/* Overlay Window */
/* Overlay window 2 */

static void _overlayer_update2(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorBlue);
    graphics_fill_rect(ctx, GRect(0, DISPLAY_ROWS - 50, DISPLAY_COLS, 50), 0, GCornerNone);
}

static void _creation_callback2(OverlayWindow *overlay_window, Window *window)
{
    _overlay_window2 = overlay_window;
    window->background_color = GColorClear;
    
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    _overlay_layer2 = layer_create(bounds);
    layer_add_child(window_layer, _overlay_layer2);
    
    layer_set_update_proc(_overlay_layer2, _overlayer_update2);
    
    overlay_window_stack_push(overlay_window, true);
    layer_mark_dirty(_overlay_layer2);
}

/* Overlay Window 2 */



// static NotificationWindow *notif_window;

static void _notif_init(OverlayWindow *overlay_window, Window *window)
{
    _overlay_window3 = overlay_window;    
    window_set_window_handlers(window, (WindowHandlers) {
        .load = _notif_test_window_load,
        .unload = _notif_test_window_unload,
    });
    
}

bool _notif_deinit(void)
{
    return true;
}

static void _notif_test_window_load(Window *window)
{    
    char *app = "RebbleOS";
    char *title = "Test Alert";
    char *body = "Testing a basic notification on RebbleOS. Create it using notification_window_create, with an app_name, title, body, and optional icon.";
    window->background_color = GColorClear;
        
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    _notif_layer = notification_layer_create(bounds);
    Notification *notification = notification_create(app, title, body, gbitmap_create_with_resource(RESOURCE_ID_SPEECH_BUBBLE), GColorRed);

    notification_layer_stack_push_notification(_notif_layer, notification);

    Notification *notification_two = notification_create("Discord", "Join Us", "Join us on the Pebble Discord in the #firmware channel", gbitmap_create_with_resource(RESOURCE_ID_ALARM_BELL_RINGING), GColorFromRGB(85, 0, 170));
    notification_layer_stack_push_notification(_notif_layer, notification_two);

    layer_add_child(layer, notification_layer_get_layer(_notif_layer));
    notification_layer_configure_click_config(_notif_layer, window, NULL);
    overlay_window_stack_push(_overlay_window3, true);

    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _notif_test_window_unload(Window *window)
{
    notification_layer_destroy(_notif_layer);
}
