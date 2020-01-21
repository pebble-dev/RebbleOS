#pragma once
/**
 * @file overlay_manager_c.h
 * @author Barry Carter
 * @date 02 Feb 2018
 * @brief RebbleOS test harness application
 *
 * Please add your test in testapp.c with your code submission
 * tests live in the SystemApp/tests folder. Put them in config.mk
 *
 * Guide:
 * once you have added your test and compiled it in, there is a lifecycle:
 * (it's much like a pebble app's lifecycle)
 * @code
 *  _init(Window *)
 *  _exec()
 *  _deinit()
 * @endcode
 * exec is where the main test is expected to happen, but any time after this
 * is considered ok.
 *
 * To complete a test use test_complete(true) which terminates it
 * immediately (and calls deinit)
 *
 * There are various asserts to use below. They will set the test state
 * appropriately and fail it is required.
 *
 * NOTES:
 * You are provided a window to draw on. Please use it.
 *
 * A test doesn't have to end. By default back fails, select is pass.
 *  - override buttons if you want
 *
 * If you test has subtests, or lots of assertions, it's up to you
 * to track the failure. There will be log entries.
 *
 * @note Once you fail a test, you can not make it pass again
 */
void testapp_main(void);
void testapp_deinit(void);
void testapp_init(void);

/**
 * @brief Get current success state.
 *
 * @return bool success or failure state of the test
 */
bool test_get_success(void);

/**
 * @brief Set current success state.
 *
 * @param bool success or failure state of the test
 * @note If the test is set to fail, it can't be a success again
 * at least until a re-run
 */
void test_set_success(bool success);

/**
 * @brief Finish up a test now.
 *
 * Complete a test right now. Wil call deinit and clean up
 * returning to the menu
 * @param bool success or failure state of the test
 */
void test_complete(bool success);

/**
 * @brief test assertion helpers
 *
 * These are test helpers to allow tests to be run safely
 * Use these to check if the draw worked, or things are null etc
 */



/**
 * @brief Generic assertion helper
 *
 * Pass in a bool of the assert you would like to track
 * @code
 * test_assert(window == NULL);
 * test_assert(window_count() == 10);
 * @endcode
 * @param pass a boolean state of the assertion
 * @return pretty much what you pass in. If you are failing a test, false. etc.
 */
bool test_assert(bool pass);

/**
 * @brief Check if a pointer is NOT NULL
 *
 * @param ptr The pointer to check the NULLality of
 * @return true if the pointer is NOT NULL
 */
bool test_assert_point_is_not_null(void *ptr);

/**
 * @brief Check if a pointer IS NULL
 *
 * @param ptr The pointer to check the NULLality of
 * @return true if the pointer IS NULL
 */
bool test_assert_point_is_null(void *ptr);

/**
 * @brief Test a \ref GPoint in the framebuffer to see if it matches a color
 *
 * Given a \ref GPoint in (x,y) on the display, go to the
 * framebuffer to see if it contains that \ref GColor, comparing for truthiness
 * @param point a \ref GPoint in x,y that you want to check the value of
 * @param color the \ref GColor to compare with the framebuffer at location
 * @return true when \ref GColor is the same as framebuffer at point
 */
bool test_assert_point_is_color(GPoint point, GColor color);

/**
 * @brief Test a \ref GPoint in the framebuffer to see if it matches
 *
 * Given a \ref GPoint in (x,y) on the display, go to the
 * framebuffer to see if it contains that value, comparing for truthiness
 * @param point a \ref GPoint in x,y that you want to check the value of
 * @param value the value you want to compare with the framebuffer at location
 * @return true when value is the same as framebuffer at point
 */
bool test_assert_point_is_value(GPoint point, uint8_t value);

/* Test declarions for various tests */
bool test_test_init(Window *window);
bool test_test_exec(void);
bool test_test_deinit(void);

bool color_test_init(Window *window);
bool color_test_exec(void);
bool color_test_deinit(void);

bool overlay_test_init(Window *window);
bool overlay_test_exec(void);
bool overlay_test_deinit(void);

bool menu_simple_test_init(Window* window);
bool menu_simple_test_exec(void);
bool menu_simple_test_deinit(void);

bool menu_multi_column_test_init(Window* window);
bool menu_multi_column_test_exec(void);
bool menu_multi_column_test_deinit(void);

bool action_menu_test_init(Window *window);
bool action_menu_test_exec(void);
bool action_menu_test_deinit(void);

bool vibes_test_init(Window *window);
bool vibes_test_exec(void);
bool vibes_test_deinit(void);

bool checkbox_test_init(Window *window);
bool checkbox_test_exec(void);
bool checkbox_test_deinit(void);

bool radio_button_test_init(Window *window);
bool radio_button_test_exec(void);
bool radio_button_test_deinit(void);

bool progress_layer_test_init(Window *window);
bool progress_layer_test_exec(void);
bool progress_layer_test_deinit(void);

bool dialog_choice_test_init(Window *window);
bool dialog_choice_test_exec(void);
bool dialog_choice_test_deinit(void);