/* service.c
 * implementation of service worker thread
 * RebbleOS
 *
 * The service worker thread is a mechanism to defer work from an interrupt
 * handler onto a FreeRTOS thread.  For instance, if database calls need to
 * take place, the service worker thread can block (for a short period of
 * time).
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "service.h"
#include "rtoswrap.h"

struct service_packet {
    service_callback_t cbk;
    void *ctx;
};

static void _service_thread(void *param);

#define SERVICE_QUEUE_SIZE 8
THREAD_DEFINE(service, configMINIMAL_STACK_SIZE + 600, tskIDLE_PRIORITY + 8UL, _service_thread);
QUEUE_DEFINE(service, struct service_packet, SERVICE_QUEUE_SIZE);

void service_init() {
    QUEUE_CREATE(service);
    THREAD_CREATE(service);
}

void service_submit(service_callback_t cbk, void *ctx) {
    struct service_packet pkt;
    pkt.cbk = cbk;
    pkt.ctx = ctx;
    
    BaseType_t woken = pdFALSE;
    
    xQueueSendFromISR(QUEUE_HANDLE(service), &pkt, &woken);
    portYIELD_FROM_ISR(woken);
}

static void _service_thread(void *params) {
    struct service_packet pkt;
    
    while(1) {
        xQueueReceive(QUEUE_HANDLE(service), &pkt, portMAX_DELAY);
        pkt.cbk(pkt.ctx);
    }
}
