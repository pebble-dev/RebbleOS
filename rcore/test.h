#pragma once
/* test.h
 * definitions for test driver
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stdint.h>

#ifdef REBBLEOS_TESTING
uint8_t test_init(void);

typedef int (*testfn_t)();
#define TESTNAME_LEN 32
struct test {
    char testname[TESTNAME_LEN];
    testfn_t testfn;
};

#define TEST(name) \
static int test_##__FILE__##_##__LINE__();\
const struct test testdef_##__FILE__##_##__LINE__ __attribute__((section(".rodata.tests"))) = { \
    .testname = #name, \
    .testfn = test_##__FILE__##_##__LINE__ \
}; \
static int test_##__FILE__##_##__LINE__()

#define TEST_PASS 0
#define TEST_FAIL -1

#else

#define TEST(name) \
static int test_##__FILE__##_##__LINE__() __attribute__((section(".throwaway")))

#endif

