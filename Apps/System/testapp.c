/* testapp.c
 * routines for Testing each feature of RebbleOS
 * See test_defs.h for more detailed information
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"
#include "platform_res.h"

static Window *s_main_window;
static Menu *s_menu;
StatusBarLayer *status_bar;

static void testapp_exec_window_unload(Window *window);
static void _reset_menu_items(void);
void _testapp_start_test_callback(void *data);

/* Some callbacks */
typedef bool (*init_test)(Window *);
typedef bool (*run_test)();

/* The test information */
typedef struct app_test_t {
    const char *test_name;
    const char *test_desc;
    init_test test_init;
    run_test test_execute;
    run_test test_deinit;
    uint16_t run_count;
    bool success;
    void *context;
} app_test;

/*TODO
 * something about hooking the buttons before we switch to the test
 * i'm sure tests will want input sometimes. Leave it to them?
 * either way stop the menu doing it's thing in the background
 *  - menu destroy/rebuild?
 *
 * move tests to test file
 */


app_test _tests[] = {
    {
        .test_name = "Test the Test",
        .test_desc = "Verify this app",
        .test_init = &test_test_init,
        .test_execute = &test_test_exec,
        .test_deinit = &test_test_deinit,
    },
    {
        .test_name = "Colour Test",
        .test_desc = "Palette",
        .test_init = &color_test_init,
        .test_execute = &color_test_exec,
        .test_deinit = &color_test_deinit,
    },
    {
        .test_name = "Overlay Test",
        .test_desc = "Window Overlay",
        .test_init = &overlay_test_init,
        .test_execute = &overlay_test_exec,
        .test_deinit = &overlay_test_deinit,
    },
    {
        .test_name = "Menu Test",
        .test_desc = "Single-Column",
        .test_init = &menu_simple_test_init,
        .test_execute = &menu_simple_test_exec,
        .test_deinit = &menu_simple_test_deinit
    },
    {
        .test_name = "Menu MC Test",
        .test_desc = "Multi-Column",
        .test_init = &menu_multi_column_test_init,
        .test_execute = &menu_multi_column_test_exec,
        .test_deinit = &menu_multi_column_test_deinit
    },
    {
        .test_name = "Action Test",
        .test_desc = "Action Menu Test",
        .test_init = &action_menu_test_init,
        .test_execute = &action_menu_test_exec,
        .test_deinit = &action_menu_test_deinit
    },
    {
        .test_name = "Vibes Test",
        .test_desc = "Vibrate Motor",
        .test_init = &vibes_test_init,
        .test_execute = &vibes_test_exec,
        .test_deinit = &vibes_test_deinit
    }
};

#define TEST_COUNT sizeof(_tests) / sizeof(app_test)
static Window *_test_window;
static AppTimer *_test_exec_timer;
static app_test *_running_test = NULL;
static bool _window_initialised = false;

/* Part of the test app's execution mechanism */

/*
 * Test signalled test completion
 * Shut down test
 */
void test_complete(bool success)
{
    SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "[%s] TC Test Complete: %s",
            success ? "PASS" : "FAIL", _running_test->test_name);
    _running_test->success = success;

    if (_test_window)
        window_destroy(_test_window);

    _test_window = NULL;

    if (_window_initialised)
    {
        _reset_menu_items();
        _window_initialised = false;
    }
}

bool test_get_success()
{
    return _running_test->success;
}

void test_set_success(bool success)
{
    if (!_running_test->success)
        return;
    SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "[%s] Test Complete: %s",
            success ? "PASS" : "FAIL", _running_test->test_name);
    _running_test->success = success;
}

/* Just flag the test as completed in whatever state it is in */
bool _test_pass(bool pass, char *msg)
{
    SYS_LOG("tstapp", pass ? APP_LOG_LEVEL_INFO : APP_LOG_LEVEL_ERROR, msg);
    test_set_success(pass);
    return pass;
}

bool test_assert(bool pass)
{
    if (pass)
        return _test_pass(true, "[ASSERT] TRUE: Assertion PASS");

    return _test_pass(false, "[ASSERT] FALSE: Assertion FAIL");
}

bool test_assert_point_is_not_null(void *ptr)
{
    if (ptr)
        return _test_pass(true, "[ASSERT] TRUE: value is NOT null");

    return _test_pass(false, "[ASSERT] FALSE: value IS null");
}

bool test_assert_point_is_null(void *ptr)
{
    if (!ptr)
    {
        return _test_pass(true, "[ASSERT] TRUE: value IS null");
    }

    return _test_pass(false, "[ASSERT] FALSE: value is NOT null");
}

/* so tempting to go UK english here... haha (ginge, UK) */
bool test_assert_point_is_color(GPoint point, GColor color)
{
    return test_assert_point_is_value(point, (uint8_t)color.argb);
}

bool test_assert_point_is_value(GPoint point, uint8_t value)
{
    uint8_t *fb = display_get_buffer();
    /* get the point, point in the fb */
    uint8_t vfb = fb[(DISPLAY_COLS * point.y) + point.x];

    if (vfb == value)
    {
        return _test_pass(true, "[ASSERT] TRUE: Point is valid");
    }

    return _test_pass(false, "[ASSERT] FALSE: Point is NOT valid");
}

/* The test's running Click handlers */

/*
 * Test was exited
 * Default behaviour is to fail
 */
void test_back_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    test_complete(false);
}

void test_select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    test_complete(true);
}

void test_up_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    SYS_LOG("tstapp", APP_LOG_LEVEL_WARNING, "Up handler not implemented");
}

void test_down_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    SYS_LOG("tstapp", APP_LOG_LEVEL_WARNING, "Down handler not implemented");
}


/*
 * Test was loaded
 */
static void testapp_exec_window_load(Window *window)
{
    assert(_running_test);
    /* Get rid of the menu */
    window_single_click_subscribe(BUTTON_ID_BACK, test_back_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, test_select_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, test_up_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, test_down_single_click_handler);

    _window_initialised = true;
    _test_exec_timer = app_timer_register(10, (AppTimerCallback)_testapp_start_test_callback, NULL);
    window_dirty(true);
}

static void testapp_exec_window_unload(Window *window)
{
    SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "Test Cleanup");
    _running_test->test_deinit(_test_window);

    SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "[%s] Test Complete: %s",
            _running_test->success ? "PASS" : "FAIL", _running_test->test_name);
    _running_test = NULL;

    if (_window_initialised)
    {
        _reset_menu_items();
        _window_initialised = false;
    }
    menu_set_click_config_onto_window(s_menu, window);
}

static MenuItems* test_test_item_selected(const MenuItem *item)
{
    app_test *test = (app_test *)item->context;

    _test_window = window_create();
    assert(_test_window);

    window_set_window_handlers(_test_window, (WindowHandlers) {
        .load = testapp_exec_window_load,
        .unload = testapp_exec_window_unload,
    });

    SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "Running Test: %s", test->test_name);
    _running_test = test;
    test->test_init(_test_window);
    test->run_count++;

    window_stack_push(_test_window, true);
    return NULL;

}

static void exit_to_watchface(struct Menu *menu, void *context)
{
    if (_running_test)
    {
        SYS_LOG("tstapp", APP_LOG_LEVEL_ERROR, "Test still running?");
        test_complete(_running_test->success);
    }

    /* Exit to watchface */
    appmanager_app_start("Simple");
}

static MenuItems *_create_menu_items(void)
{
    MenuItems *items = menu_items_create(TEST_COUNT);

    for(int i = 0; i < TEST_COUNT; i++)
    {
        /* choose our icon */
        uint16_t icon = RESOURCE_ID_CLOCK;
        if (_tests[i].run_count > 0)
        {
            if (_tests[i].success)
                icon = RESOURCE_ID_MUSIC_PLAY;
            else
                icon = RESOURCE_ID_MUSIC_PAUSE;
        }

        _tests[i].context = &icon;
        MenuItem mi = MenuItem((char *)_tests[i].test_name, (char *)_tests[i].test_desc, icon, test_test_item_selected);
        mi.context = &_tests[i];
        menu_items_add(items, mi);
    }

    return items;
}

static void _reset_menu_items(void)
{
    MenuItems *back = s_menu->items->back;
    MenuIndex *index = &s_menu->items->back_index;

    MenuItems *items = _create_menu_items();
    menu_set_items(s_menu, items);

    items->back = back;
    items->back_index = *index;
}

static void testapp_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_RECT
    s_menu = menu_create(GRect(0, 16, DISPLAY_COLS, DISPLAY_ROWS - 16));
#else
    // Let the menu draw behind the statusbar so it is perfectly centered
    s_menu = menu_create(GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS));
#endif
    menu_set_callbacks(s_menu, s_menu, (MenuCallbacks) {
        .on_menu_exit = exit_to_watchface
    });
    layer_add_child(window_layer, menu_get_layer(s_menu));
    menu_set_click_config_onto_window(s_menu, window);

    _reset_menu_items();

    // Status Bar
    status_bar = status_bar_layer_create();
    layer_add_child(menu_get_layer(s_menu), status_bar_layer_get_layer(status_bar));
}

void _testapp_start_test_callback(void *data)
{
    SYS_LOG("tstapp", APP_LOG_LEVEL_INFO, "Executing test");
    app_timer_cancel(_test_exec_timer);
    _running_test->success = true;
    _running_test->test_execute();

}

static void testapp_window_unload(Window *window)
{
    menu_destroy(s_menu);
}

void testapp_init(void)
{
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = testapp_window_load,
        .unload = testapp_window_unload,
    });

    window_stack_push(s_main_window, true);
}

void testapp_deinit(void)
{
    for(int i = 0; i < TEST_COUNT; i++)
    {
        /* clean up the icon context */
        if (_tests[i].context)
        {
            app_free(_tests[i].context);
            _tests[i].context = NULL;
        }
    }
    window_destroy(s_main_window);
}

void testapp_main(void)
{
    testapp_init();
    app_event_loop();
    testapp_deinit();
}
