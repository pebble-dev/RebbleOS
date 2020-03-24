#include "test.h"
#include "rebbleos.h"
#include "protocol.h"
#include "protocol_service.h"
#include "rtoswrap.h"

#ifndef REBBLEOS_TESTING
#error these routines only get compiled in test mode
#endif

enum qemu_test_packet_types {
    QEMU_TEST_LIST_REQUEST  = 0x0000,
    QEMU_TEST_RUN_REQUEST   = 0x0001,
    QEMU_TEST_LIST_RESPONSE = 0x8000,
    QEMU_TEST_COMPLETE      = 0x8001,
    QEMU_TEST_ALIVE         = 0xFFFF,
};

#define RESPONSE_NAME_MAX_SIZE 256
struct qemu_test_list_response {
    uint16_t opcode;
    uint16_t id;
    uint8_t is_last_test;
    uint8_t name_len;
    char name[RESPONSE_NAME_MAX_SIZE];
} __attribute__((__packed__)); 

struct qemu_test_run_request {
    uint16_t opcode;
    uint16_t id;
} __attribute__((__packed__));

struct qemu_test_complete {
    uint16_t opcode;
    uint8_t passed;
    uint32_t artifact;
} __attribute__((__packed__));

union qemu_test_packet {
    uint16_t opcode;
    struct qemu_test_list_response test_list_response;
    struct qemu_test_run_request test_run_request;
    struct qemu_test_complete test_complete;
} __attribute__((__packed__));

extern struct test __tests_start[];
extern struct test __tests_end[];

static void _test_thread(void *par);
THREAD_DEFINE(test, 1024, tskIDLE_PRIORITY + 2UL, _test_thread);
QUEUE_DEFINE(test, uint16_t, 1);

uint8_t test_init() {
    struct test *test;
    int i;
    
    QUEUE_CREATE(test);
    THREAD_CREATE(test);
    
    for (test = __tests_start; test < __tests_end; test++) {
        SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "Test \"%s\" available", test->testname);
    }
    return INIT_RESP_OK;
}

static void _test_thread(void *par) {
    uint16_t alive;
    alive = htons(QEMU_TEST_ALIVE);
    qemu_reply_test(&alive, sizeof(alive)); /* Tell QEMU we are awake. */
    
    appmanager_test_become_thread(AppThreadMainApp);
    
    while (1) {
        uint16_t testreq;
        static struct qemu_test_complete resp;
        resp.opcode = htons(QEMU_TEST_COMPLETE);
        
        xQueueReceive(_test_queue, &testreq, portMAX_DELAY); /* does not fail */
        
        if (&__tests_start[testreq] >= __tests_end) {
            KERN_LOG("test", APP_LOG_LEVEL_ERROR, "request for out-of-bound test %d", testreq);
            
            resp.passed = 0;
            resp.artifact = htonl(0xBAADF00DU);
            qemu_reply_test(&resp, sizeof(resp));
            continue;
        }
        
        KERN_LOG("test", APP_LOG_LEVEL_INFO, "running test %d (%s)", testreq, __tests_start[testreq].testname);
        
        uint32_t artifact;
        int rv;
        
        rv = __tests_start[testreq].testfn(&artifact);
        
        resp.passed = rv == TEST_PASS;
        resp.artifact = htonl(artifact);
        qemu_reply_test(&resp, sizeof(resp));
    }
}

void test_packet_handler(const RebblePacket packet)
{
    union qemu_test_packet *qpkt = (union qemu_test_packet *)packet_get_data(packet);
    
    KERN_LOG("test", APP_LOG_LEVEL_INFO, "test protocol packet from qemu, %d bytes, opcode %d", packet_get_data_length(packet), qpkt->opcode);
    
    switch (ntohs(qpkt->opcode)) {
    case QEMU_TEST_LIST_REQUEST: {
        static struct qemu_test_list_response resp;
        int i;
        struct test *test;
        
        for (test = __tests_start, i = 0; test < __tests_end; test++, i++) {
            resp.opcode = htons(QEMU_TEST_LIST_RESPONSE);
            resp.id = htons(i);
            resp.is_last_test = test == (__tests_end - 1);
            resp.name_len = strnlen(test->testname, RESPONSE_NAME_MAX_SIZE);
            strncpy(resp.name, test->testname, RESPONSE_NAME_MAX_SIZE);
            
            KERN_LOG("test", APP_LOG_LEVEL_INFO, "replying with test %s", test->testname);
            qemu_reply_test(&resp, sizeof(resp) - RESPONSE_NAME_MAX_SIZE + resp.name_len);
        }
        
        break;
    }
    case QEMU_TEST_RUN_REQUEST: {
        uint16_t testreq = ntohs(qpkt->test_run_request.id);
        
        if (!xQueueSendToBack(_test_queue, &testreq, 0))
            KERN_LOG("test", APP_LOG_LEVEL_ERROR, "test engine was busy");
        
        break;
    }
    default:
        KERN_LOG("test", APP_LOG_LEVEL_ERROR, "unknown qemu opcode %04x", qpkt->opcode);
    }
}

TEST(simple) {
    *artifact = 42;
    SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "simple test passes");
    return TEST_PASS;
}
