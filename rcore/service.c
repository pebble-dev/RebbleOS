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
#include "rebble_memory.h"
#include "log.h"

struct service_packet {
    service_callback_t cbk;
    void *ctx;
    TickType_t when;
    struct service_packet *next;
};

static void _service_thread(void *param);

#define SERVICE_QUEUE_SIZE 8
THREAD_DEFINE(service, configMINIMAL_STACK_SIZE + 600, tskIDLE_PRIORITY + 8UL, _service_thread);
QUEUE_DEFINE(service, struct service_packet, SERVICE_QUEUE_SIZE);

void service_init() {
    QUEUE_CREATE(service);
    THREAD_CREATE(service);
}

static struct service_packet *pktq = NULL;

void service_submit(service_callback_t cbk, void *ctx, uint32_t when) {
    struct service_packet pkt;
    pkt.cbk = cbk;
    pkt.ctx = ctx;
    pkt.when = xTaskGetTickCount() + when;
    
    BaseType_t woken = pdFALSE;
    
    xQueueSendFromISR(QUEUE_HANDLE(service), &pkt, &woken);
    portYIELD_FROM_ISR(woken);
}

static void _service_thread(void *params) {
    struct service_packet pkt;
    
    while(1) {
        TickType_t whennext;
        if (!pktq)
            whennext = portMAX_DELAY;
        else
            whennext = (xTaskGetTickCount() >= pktq->when) ? 0 : (pktq->when - xTaskGetTickCount());
        
        if (xQueueReceive(QUEUE_HANDLE(service), &pkt, whennext)) {
            /* Insert the new packet. */
            struct service_packet *mpkt = malloc(sizeof(struct service_packet));
            if (!mpkt) {
                SYS_LOG("svc", APP_LOG_LEVEL_ERROR, "OOM allocating service packet");
                continue;
            }
            memcpy(mpkt, &pkt, sizeof(pkt));
            
            /* Find a place to put it in the queue. */
            struct service_packet **nextp = &pktq;
            while (*nextp && (*nextp)->when < pkt.when)
                nextp = &((*nextp)->next);
            mpkt->next = *nextp;
            *nextp = mpkt;
        }
        
        /* Finally, do any work that needs doing. */
        while (pktq && pktq->when <= xTaskGetTickCount()) {
            struct service_packet *cpkt = pktq;
            cpkt->cbk(cpkt->ctx);
            pktq = cpkt->next;
            free(cpkt);
        }
    }
}
