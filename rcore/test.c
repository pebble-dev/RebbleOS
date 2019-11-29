#include "test.h"
#include "rebbleos.h"

#ifndef REBBLEOS_TESTING
#error these routines only get compiled in test mode
#endif

extern struct test __tests_start[];
extern struct test __tests_end[];

uint8_t test_init() {
    struct test *test;
    int i;
    
    for (test = __tests_start; test < __tests_end; test++) {
        SYS_LOG("Test", APP_LOG_LEVEL_DEBUG, "Test \"%s\" available", test->testname);
    }
    return INIT_RESP_OK;
}

TEST(simple) {
    return TEST_PASS;
}