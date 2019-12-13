#pragma once
/* test.h
 * definitions for test driver
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stdint.h>

#ifdef REBBLEOS_TESTING
#include "protocol.h"

uint8_t test_init(void);
void test_packet_handler(const pbl_transport_packet *packet);

typedef int (*testfn_t)(uint32_t *artifact);
#define TESTNAME_LEN 32
struct test {
    char testname[TESTNAME_LEN];
    testfn_t testfn;
};

#define TEST(name) \
static int test_##name(uint32_t *artifact);\
const struct test testdef_##name __attribute__((section(".rodata.tests"))) = { \
    .testname = #name, \
    .testfn = test_##name \
}; \
static int test_##name(uint32_t *artifact)

#define TEST_PASS 0
#define TEST_FAIL -1

#else

#define TEST(name) \
static int test_## #name(uint32_t artifact) __attribute__((section(".throwaway")))

#endif

