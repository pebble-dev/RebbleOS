#include "rebbleos.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"

bool test_test_init(Window *window)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "Init: Test Test");
    return true;
}

bool test_test_exec(void)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "Exec: Test Test");
    /* lets do some asserting */
    test_assert(1 == 1);

    /* Let's set to true */
    test_complete(true);
    return true;
}

bool test_test_deinit(void)
{
    APP_LOG("test", APP_LOG_LEVEL_ERROR, "De-Init: Test Test");
    return true;
}
