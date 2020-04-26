/* ppogatt.c
 * Pebble Protocol over GATT (Bluetooth LE) implementation
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 *
 * The PPoGATT layer lies between the Bluetooth LE stack interface (which
 * handles the nuance of configuring the BLE peripheral itself, setting up
 * the GATT connection, handling NOTIFY / READ / WRITE characteristic
 * events, doing advertising, etc., etc., etc.) and the Pebble Protocol
 * implementation ("bluetooth.c"), which generates high-level Pebble
 * packets.
 *
 * PPoGATT is, at its essence, an implementation of a reliable in-order
 * protocol on top of an otherwise unreliable and unordered transport.  It
 * uses WRITE COMMAND and NOTIFY operations, depending on which end is
 * client and which end is server; those operations do not have
 * acknowledgements, so PPoGATT layers acknowledgements inside of its own
 * protocol.
 *
 * The PPoGATT protocol is a relatively simple shim around the Pebble
 * Protocol.  The first byte of a PPoGATT packet is a bitfield:
 *   data[7:0] = {seq[4:0], cmd[2:0]}
 *
 * cmd can have four values that we know of:
 *
 *   3'd0: Data packet with sequence `seq`.  Should be responded to with an
 *         ACK packet with the same sequence.  If a packet in sequence is
 *         missing, do not respond with any ACKs until the missing sequenced
 *         packet is retransmitted.
 *   3'd1: ACK for data packet with sequence `seq`.
 *   3'd2: Reset request. [has data unknown]
 *   3'd3: Reset ACK. [has data unknown]
 *
 * Sequences are increasing and repeating.
 */

#include <debug.h>
#include "platform.h"
#include "FreeRTOS.h"
#include "service.h"
#include "rebble_memory.h"
#include "task.h" /* xTaskCreate */
#include "queue.h" /* xQueueCreate */
#include "log.h" /* KERN_LOG */
#include "rbl_bluetooth.h"

#ifdef BLUETOOTH_IS_BLE

enum ppogatt_cmd {
    PPOGATT_CMD_DATA = 0,
    PPOGATT_CMD_ACK = 1,
    PPOGATT_CMD_RESET_REQ = 2,
    PPOGATT_CMD_RESET_ACK = 3,
};

#define STACK_SIZE_PPOGATT_RX (configMINIMAL_STACK_SIZE + 600)
#define STACK_SIZE_PPOGATT_TX (configMINIMAL_STACK_SIZE + 600)

static TaskHandle_t _task_ppogatt_rx = 0;
static StaticTask_t _task_ppogatt_rx_tcb;
static StackType_t  _task_ppogatt_rx_stack[STACK_SIZE_PPOGATT_RX];

static TaskHandle_t _task_ppogatt_tx = 0;
static StaticTask_t _task_ppogatt_tx_tcb;
static StackType_t  _task_ppogatt_tx_stack[STACK_SIZE_PPOGATT_TX];

/* XXX: could be optimized to save memory and support more outstanding
 * packets by allocating from a variable-sized pool, so we don't waste a
 * whole queue entry when we potentially only need an ACK's worth of data */

#define PPOGATT_MTU 256

/* XXX: need to do MTU detection */
#define PPOGATT_TX_MTU 20

#define PPOGATT_RX_QUEUE_SIZE 8
#define PPOGATT_TX_QUEUE_SIZE 4

struct ppogatt_packet {
    uint32_t len;
    uint8_t buf[PPOGATT_MTU];
};

static QueueHandle_t         _queue_ppogatt_rx = 0;
static StaticQueue_t         _queue_ppogatt_rx_qcb;
static struct ppogatt_packet _queue_ppogatt_rx_buf[PPOGATT_RX_QUEUE_SIZE];

static QueueHandle_t         _queue_ppogatt_tx = 0;
static StaticQueue_t         _queue_ppogatt_tx_qcb;
static struct ppogatt_packet _queue_ppogatt_tx_buf[PPOGATT_TX_QUEUE_SIZE];

static int pktslost = 0;

static void _ppogatt_rx_main(void *param) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: rx thread awake");
    mem_thread_set_heap(&mem_heaps[HEAP_LOWPRIO]);

    while (1) {
        static struct ppogatt_packet pkt;
        
        xQueueReceive(_queue_ppogatt_rx, &pkt, portMAX_DELAY); /* does not fail, since we wait forever */
        
        if (pktslost) {
            pktslost = 0;
            DRV_LOG("bt", APP_LOG_LEVEL_ERROR, "rx: packet lost!");
        }
        
        uint8_t cmd = pkt.buf[0] & 7;
        uint8_t seq = pkt.buf[0] >> 3;
        
        switch (cmd) {
        case PPOGATT_CMD_DATA:
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: data seq %d", seq);
            bluetooth_data_rx(pkt.buf + 1, pkt.len - 1);
            
            /* Send an ACK (not that Gadgetbridge cares... */
            /* XXX: needs to be part of the Big Tx State Machine */
            pkt.len = 1;
            pkt.buf[0] = seq | PPOGATT_CMD_ACK;
            xQueueSendToBack(_queue_ppogatt_tx, &pkt, portMAX_DELAY);

            break;
        case PPOGATT_CMD_ACK:
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: ack seq %d", seq);
            break;
        case PPOGATT_CMD_RESET_REQ:
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: RESET REQ seq %d", seq);
            break;
        case PPOGATT_CMD_RESET_ACK:
            DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: RESET ACK seq %d", seq);
            break;
        }
    }
}

/* XXX: PPoGATT TX thread probably doesn't have a ppogatt_packet queue, but
 * instead has a produce/consume buffer, and also a ACK-needed / ACK-sent
 * pair of chasing counters.  It prioritizes sending an ACK if one is
 * needed.  (Note that ACK-needed is both a counter and a flag; if a datum
 * is retransmitted, even if we think we've sent an ACK, they might not have
 * heard it, so we'd bump the ACK-needed flag without incrementing the
 * counter.)
 *
 * The produce side of the data produce/consume makes sense, but the consume
 * has multiple pointers for various sequence numbers past.  Note that the
 * *rx* thread bumps forward the consume pointers once ACKs come back in. 
 * The TX thread also probably needs to remember when it needs to
 * retransmit the outstanding packets...
 */

static void _ppogatt_tx_main(void *param) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "tx: tx thread awake");

    while (1) {
        static struct ppogatt_packet pkt;
        int rv;
        
        xQueueReceive(_queue_ppogatt_tx, &pkt, portMAX_DELAY); /* does not fail, since we wait forever */
        
        while (ble_ppogatt_tx(pkt.buf, pkt.len) < 0) {
            rv = ulTaskNotifyTake(pdTRUE /* clear on exit */, pdMS_TO_TICKS(250) /* try again, even if the stack wedges */);
            if (rv == 0) {
                DRV_LOG("bt", APP_LOG_LEVEL_ERROR, "warning: BLE stack did not notify TX ready?");
            }
        }
//        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "hw tx: did tx %d bytes, %02x %02x %02x...", pkt.len, pkt.buf[0], pkt.buf[1], pkt.buf[2]);
    }
}

static void _ppogatt_callback_txready() {
    BaseType_t woken = pdFALSE;
    
    vTaskNotifyGiveFromISR(_task_ppogatt_tx, &woken);
    
    portYIELD_FROM_ISR(woken);
}

static struct ppogatt_packet _ppogatt_rx_msg;

static void _ppogatt_callback_rx(const uint8_t *buf, size_t len) {
    BaseType_t woken = pdFALSE;
    
    memcpy(_ppogatt_rx_msg.buf, buf, len);
    _ppogatt_rx_msg.len = len;
    
    /* If it fails, we'll retransmit later -- ignore the return. */
    int rv;
    rv = xQueueSendFromISR(_queue_ppogatt_rx, &_ppogatt_rx_msg, &woken);
    
    if (!rv)
        pktslost++;
    
    portYIELD_FROM_ISR(woken);
}

static void _ppogatt_shutdown() {
    /* Shut down anything pending before we start clearing queues. */
    if (_task_ppogatt_rx) {
        vTaskDelete(_task_ppogatt_rx);
        _task_ppogatt_rx = 0;
    }
    
    if (_task_ppogatt_tx) {
        vTaskDelete(_task_ppogatt_tx);
        _task_ppogatt_rx = 0;
    }
    
    /* Kill off the queues. */
    if (_queue_ppogatt_rx) {
        QueueHandle_t oldq = _queue_ppogatt_rx;
        _queue_ppogatt_rx = 0;
        vQueueDelete(oldq);
    }

    if (_queue_ppogatt_tx) {
        QueueHandle_t oldq = _queue_ppogatt_tx;
        _queue_ppogatt_tx = 0;
        vQueueDelete(oldq);
    }
}

static void _ppogatt_start() {
    _ppogatt_shutdown();

    /* Create new queues. */
    _queue_ppogatt_rx = xQueueCreateStatic(PPOGATT_RX_QUEUE_SIZE, sizeof(struct ppogatt_packet), (void *)_queue_ppogatt_rx_buf, &_queue_ppogatt_rx_qcb);
    _queue_ppogatt_tx = xQueueCreateStatic(PPOGATT_TX_QUEUE_SIZE, sizeof(struct ppogatt_packet), (void *)_queue_ppogatt_tx_buf, &_queue_ppogatt_tx_qcb);
    
    /* Start up the PPoGATT tasks. */
    _task_ppogatt_rx = xTaskCreateStatic(_ppogatt_rx_main, "PPoGATT rx", STACK_SIZE_PPOGATT_RX, NULL, tskIDLE_PRIORITY + 4UL, _task_ppogatt_rx_stack, &_task_ppogatt_rx_tcb);
    _task_ppogatt_tx = xTaskCreateStatic(_ppogatt_tx_main, "PPoGATT tx", STACK_SIZE_PPOGATT_TX, NULL, tskIDLE_PRIORITY + 4UL, _task_ppogatt_tx_stack, &_task_ppogatt_tx_tcb);
}

static void _ppogatt_callback_connected(void *ctx) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT connect");

    _ppogatt_start();
    
    /* Send a wakeup packet. */
    static struct ppogatt_packet pkt;
    
    pkt.len = 2;
    pkt.buf[0] = 0x02;
    pkt.buf[1] = 0x00;
    xQueueSendToBack(_queue_ppogatt_tx, &pkt, portMAX_DELAY);
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT start message enqueued");
    
    bluetooth_device_connected();
}

static void _ppogatt_callback_connected_isr() {
    service_submit(_ppogatt_callback_connected, NULL);
}

static void _ppogatt_callback_disconnected(void *ctx) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "PPoGATT disconnect");
    _ppogatt_shutdown();
    bluetooth_device_disconnected();
}

static void _ppogatt_callback_disconnected_isr() {
    service_submit(_ppogatt_callback_disconnected, NULL);
}

/* Main entry for PPoGATT code, to be called at boot. */
void ppogatt_init() {
    /* Point the ISRs at us. */
    ble_ppogatt_set_callback_connected(_ppogatt_callback_connected_isr);
    ble_ppogatt_set_callback_rx(_ppogatt_callback_rx);
    ble_ppogatt_set_callback_txready(_ppogatt_callback_txready);
    ble_ppogatt_set_callback_disconnected(_ppogatt_callback_disconnected_isr);
}

/*** PPoGATT <-> BT stack ***/

uint8_t txseq = 0;

void bt_device_request_tx(uint8_t *data, uint16_t len) {
    /* XXX: needs to be part of the Big Tx State Machine */
    static struct ppogatt_packet pkt;

    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "OS TX: OS wanted us to tx %d bytes", len);
    
    while (len > 0) {
        uint16_t thislen = (len > (PPOGATT_TX_MTU - 1)) ? (PPOGATT_TX_MTU - 1) : len;
        
        pkt.len = thislen + 1;
        pkt.buf[0] = (txseq << 3) | PPOGATT_CMD_DATA;
        memcpy(pkt.buf + 1, data, thislen);
        xQueueSendToBack(_queue_ppogatt_tx, &pkt, portMAX_DELAY);
        
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "OS TX: did tx seq %d with %d bytes", txseq, thislen + 1);
        
        len -= thislen;
        data += thislen;
        
        txseq++;
    }
    
    bluetooth_tx_complete_from_isr();
}

#endif
