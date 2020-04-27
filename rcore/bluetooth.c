/* bluetooth.c
 * routines for controlling a bluetooth stack
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
/*
 * General flow:
 * 
 * TX
 * A packet is genreated and posted to the cmd thread
 * btstack_rebble takes a ref to this data,
 * and waits for a ready to send message fromt he bt stack.
 *  * NOTE the memory is not copied, it is sent with supplied buf
 * 
 * RX
 * An incoming packet is collected in btstack_rebble
 * once we have some data, we look over the buffer and check for packet header
 * If we have packets, we recombine them and process as required.
 *  
 * NOTES:
 * we have locking and whatnot. Test it.
 * determine max MTU size (eats ram)        
 */

#include "FreeRTOS.h"
#include "task.h" /* xTaskCreate */
#include "timers.h" /* xTimerCreate */
#include "queue.h" /* xQueueCreate */
#include "platform.h" /* hw_backlight_set */
#include "log.h" /* KERN_LOG */
#include "backlight.h"
#include "ambient.h"
#include "rebble_memory.h"
#include "rebbleos.h"
#include "rbl_bluetooth.h"
#include "pebble_protocol.h"
#include "stdarg.h"
#include "connection_service.h"
#include "protocol.h"
#include "protocol_service.h"
#include "service.h"
#include "rdb.h"

/* Stack sizes of the threads */
#define STACK_SZ_CMD configMINIMAL_STACK_SIZE + 600
#define STACK_SZ_BT 1800

/* Bit commands for the binary semaphore */
#define TX_NOTIFY_COMPLETE 1

extern int vsfmt(char *buf, unsigned int len, const char *ifmt, va_list ap);


/* a packet to go on the queue to process a command */
typedef struct rebble_bt_packet_t {
    uint8_t packet_type;
    size_t length;
    uint8_t *data;
    tx_complete_callback callback;
} rebble_bt_packet;


/* BT runloop */
static TaskHandle_t _bt_task;
static StackType_t _bt_task_stack[STACK_SZ_BT];
static StaticTask_t _bt_task_buf;

/* for the command processer */
static TaskHandle_t _bt_cmd_task;
static StackType_t _bt_cmd_task_stack[STACK_SZ_CMD];
static StaticTask_t _bt_cmd_task_buf;

/* Processing Queue */
#define _CMD_QUEUE_LENGTH 5
#define _CMD_QUEUE_SIZE sizeof(rebble_bt_packet)
static QueueHandle_t _bt_cmd_queue;
static StaticQueue_t _bt_cmd_queue_ptr;
static uint8_t _bt_cmd_queue_buf[_CMD_QUEUE_LENGTH * _CMD_QUEUE_SIZE];

static SemaphoreHandle_t _bt_tx_mutex;
static StaticSemaphore_t _bt_tx_mutex_buf;


static bool _enabled;
static bool _connected;


/* Packet command types */
#define PACKET_TYPE_RX 0
#define PACKET_TYPE_TX 1

static void _bt_thread(void *pvParameters);
static void _bt_cmd_thread(void *pvParameters);
static uint8_t _bluetooth_tx(uint8_t *data, uint16_t len);

static void _get_bond_data_isr(const void *peer, size_t len);
static void _request_bond_isr(const void *peer, size_t len, const char *name, const void *data, size_t datalen);

 #define BT_LOG_ENABLED
#ifdef BT_LOG_ENABLED
    #define BT_LOG SYS_LOG
#else
    #define BT_LOG NULL_LOG
#endif

#define MODULE_NAME "bt"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE

/* Initialise the bluetooth module */
uint8_t bluetooth_init(void)
{
    _bt_cmd_queue = xQueueCreateStatic(_CMD_QUEUE_LENGTH, _CMD_QUEUE_SIZE, _bt_cmd_queue_buf, &_bt_cmd_queue_ptr);
    _bt_tx_mutex = xSemaphoreCreateMutexStatic(&_bt_tx_mutex_buf);
    _bt_task = xTaskCreateStatic(_bt_thread, 
                                     "BT", STACK_SZ_BT, NULL, 
                                     tskIDLE_PRIORITY + 5UL, 
                                     _bt_task_stack, &_bt_task_buf);
    
    _bt_cmd_task = xTaskCreateStatic(_bt_cmd_thread, 
                                     "BTCmd", STACK_SZ_CMD, NULL, 
                                     tskIDLE_PRIORITY + 6UL, 
                                     _bt_cmd_task_stack, &_bt_cmd_task_buf);

#ifdef BLUETOOTH_IS_BLE
    ppogatt_init();
#endif

    bt_set_callback_get_bond_data(_get_bond_data_isr);
    bt_set_callback_request_bond(_request_bond_isr);
    
    return INIT_RESP_ASYNC_WAIT;
}

void bluetooth_init_complete(uint8_t state)
{
    _enabled = (state != INIT_RESP_OK ? false : true);
    os_module_init_complete(state);
}


/*
 * Callbacks and ISR from the bluetooth stack
 */


/*
 * Some data arrived from the stack
 */
void bluetooth_data_rx(uint8_t *data, size_t len)
{
    LOG_DEBUG("bluetooth_data_rx: %d bytes", len);
    
    if (protocol_rx_buffer_append(data, len) == PROTOCOL_BUFFER_FULL) {
        LOG_ERROR("protocol buffer was full... dropping some data");
        return;
    }
    
    RebblePacket packet = packet_create_with_data(0, NULL, 0); /* data has no meaning here */
    packet_set_transport(packet, bluetooth_send_data);
    assert(packet);
    packet_recv(packet); /* kick the protocol thread into action */
}

/*
 * We sent a packet. it was sent.
 */
void bluetooth_tx_complete_from_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(_bt_cmd_task, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


/* 
 * Threads
 */

/*
 * Bluetooth thread. The device is initialised here
 * 
 * XXX move freertos runloop code to here?
 */
static void _bt_thread(void *pvParameters)
{
    mem_thread_set_heap(&mem_heaps[HEAP_LOWPRIO]);
    
    /* We are blocked here while bluetooth further delegates a runloop */
    hw_bluetooth_init();

    /* Delete ourself and die */
    vTaskDelete(_bt_task);
    return;
}

/* TODO doesn't really need a thread here 
 * a simple tx() { wait for mutex }
 */
static void _bt_cmd_thread(void *pvParameters)
{
    static rebble_bt_packet pkt;
    BT_LOG("BT", APP_LOG_LEVEL_INFO, "BT CMD Thread started");
    
    for( ;; )
    {
        /* We are going to wait for a message. This is mostly going to be TX messages
         * We have a queue size of one, which blocks and serialises all callers
         */
        xQueueReceive(_bt_cmd_queue, &pkt, portMAX_DELAY);
        
        if (pkt.packet_type == PACKET_TYPE_TX)
        {
            bt_device_request_tx(pkt.data, pkt.length);
            int rv = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
            if(rv == 0)
                BT_LOG("BT", APP_LOG_LEVEL_ERROR, "Timed out sending!");

            /* XXX ugh, should be using a better dynamic pool of memory not sys malloc */
            remote_free(pkt.data);
        }
    }
}

/* 
 * Utility
 */

void bluetooth_send_data(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    uint16_t preamble[2] = {
        htons(len), htons(endpoint)
    };

    bluetooth_send((uint8_t *)preamble, 4);
    bluetooth_send(data, len);
}

/* 
 * Request a TX through the outboud thread mechanism
 */
void bluetooth_send_async(uint8_t *data, size_t len, tx_complete_callback cb)
{
    static rebble_bt_packet packet;
    
    if (!_enabled || !_connected)
        return;
    
    /* XXX ugh, should be using a better dynamic pool of memory not sys malloc */
    uint8_t *ndata = malloc(len);
    assert(ndata && "Well that will teach you for letting me abuse malloc. Fix this code");
    memcpy(ndata, data, len);
    
    packet.length = len;
    packet.packet_type = PACKET_TYPE_TX;
    packet.data = ndata;
    packet.callback = cb;
    
    xQueueSendToBack(_bt_cmd_queue, &packet, portMAX_DELAY);
}
    
inline uint8_t bluetooth_send(uint8_t *data, size_t len)
{
    if (!_enabled || !_connected)
        return len;

    return _bluetooth_tx(data, len);
}

void bluetooth_device_connected(void)
{
    _connected = true;
    protocol_set_current_transport_sender(bluetooth_send_data);
    connection_service_update(true);
    
    /* Forge an rx ping to set the transport, and to transmit real data
     * over the link (a pong).  */
    static const uint8_t appversion_req[] = {
        0x00
    };
    RebblePacket pkt = packet_create_with_data(WatchProtocol_AppVersion, appversion_req, sizeof(appversion_req));
    packet_send(pkt);

#if 0    
    static const uint8_t legacyapp_data[] = {
        0x01, 0x02, 0x0c, 0x3c, 0xa0, 0xae, 0x90, 0x67, 0x47, 0x64, 0xaf, 0x92,
        0xbd, 0xca, 0xd1, 0xfb, 0x60, 0x84, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00
    };
    pkt = packet_create_with_data(0x31, legacyapp_data, sizeof(legacyapp_data));
    packet_send(pkt);
#endif
}

void bluetooth_device_disconnected(void)
{
    _connected = false;
    connection_service_update(false);
}

bool bluetooth_is_device_connected(void)
{
    return _connected;
}

bool bluetooth_is_enabled(void)
{
    return _enabled;
}

void bluetooth_enable(void)
{
    _enabled = true;
}


static uint8_t _bluetooth_tx(uint8_t *data, uint16_t len)
{
    BaseType_t result;
    uint32_t notif_value;
    
    xSemaphoreTake(_bt_tx_mutex, portMAX_DELAY);

    /* We need to break the tx into chunks that fit into the MTU size
     * rfcomm also needs a little room for the header */
    uint32_t sz = HCI_ACL_PAYLOAD_SIZE - 10;
    
    while(len > 0)
    {           
        uint32_t thissendlen = len > sz ? sz : len;
        bluetooth_send_async(data, thissendlen, NULL);

        len  -= thissendlen;
        data += thissendlen;
    }

    xSemaphoreGive(_bt_tx_mutex);
    return len;
}



// Ugh compat with btstack
// XXX So hack
int sprintf(char *str, const char *format, ...)
{
    va_list ap;
    int n;

    va_start(ap, format);
    n = vsfmt(str, 128, format, ap);
    va_end(ap);

    return n;
}




int sscanf ( const char * s, const char * format, ...)
{
    return 0;
}

/* Bond database management. */

/* XXX: This flow will all change soon.  On a Pebble, you get the name
 * during a legacy pairing request, but this is not how it works on Pebble
 * 2, which is LE only.  The Pebble legacy flow is:
 *
 *   Phone initiates pairing.
 *   Pebble displays "Pair? [phone name]" on screen (and maybe a passkey, on old firmware).
 *   Pebble user clicks 'yes'; Pebble shows "pairing...".
 *   Phone confirms pairing.
 *   Pebble shows pairing is complete.
 *
 * This flow is what is implemented here, but isn't really representative of
 * how BLE bonding works.  In BLE pairing, we do not get a peer name
 * associated with a bonding request -- and, interestingly, the Pebble seems
 * to initiate the bonding request!  The BLE flow, on a Pebble 2, looks like
 * this:
 *
 *   User initiates pairing:
 *     Phone connects by BLE, and writes to pairing characteristic.
 *   Pebble displays "pair?" screen, without peer name.
 *   Pebble user clicks "yes"; Pebble shows "pairing...".
 *     Pebble responds by initiating bonding request.  (Note that bonding has started before user taps "yes"!)
 *   Phone confirms pairing.
 *     Phone and Pebble exchange bond information.
 *     Only now does Pebble look up peer's name (iPhones lie about their name until bonded!).
 *     Pebble stores keys in bond database.
 *   Pebble displays that pairing is complete.
 * 
 * We will need to change the API so that "store bond data" is an
 * unconditional command that signals successful bonding, that there is a
 * "bond attempt failed" callback, and that the "request to peer" command
 * comes with an optional name, and not with keys.
 */

static int _bondreq_in_progress = 0;
static const void *_bondreq_peer;
static int _bondreq_len;
static const void *_bondreq_name;
static const void *_bondreq_data;
static int _bondreq_datalen;

static void _get_bond_data(void *p) {
    struct rdb_database *db = rdb_open(RDB_ID_BLUETOOTH);
    struct rdb_iter it;
    rdb_select_result_list head = LIST_HEAD(head);
    
    int rv = rdb_iter_start(db, &it);
    if (!rv) {
        DRV_LOG("btbond", APP_LOG_LEVEL_INFO, "no bond database (looking up \"%08x...\")", *(uint32_t *)_bondreq_peer);
        hw_bluetooth_bond_data_available(NULL, 0);
        rdb_close(db);
        return;
    }

    struct rdb_selector selectors[] = {
        { RDB_SELECTOR_OFFSET_KEY, _bondreq_len, RDB_OP_EQ, (void *) _bondreq_peer },
        { 0, 0, RDB_OP_RESULT_FULLY_LOAD },
        { }
    };
    
    int n = rdb_select(&it, &head, selectors);
    assert(n <= 1 && "must have zero or one bluetooth bond results...");
    if (n == 0) {
        DRV_LOG("btbond", APP_LOG_LEVEL_INFO, "no bond data available for \"%08x...\"", *(uint32_t *)_bondreq_peer);
        hw_bluetooth_bond_data_available(NULL, 0);
        rdb_close(db);
        return;
    }
    
    struct rdb_select_result *res = rdb_select_result_head(&head);
    uint8_t namelen = ((uint8_t *)res->result[0])[0] /* including terminating 0 */;
    uint8_t bondlen = ((uint8_t *)res->result[0])[1];
    
    DRV_LOG("btbond", APP_LOG_LEVEL_INFO, "found bond (res %p, result %p) for device %s with len %d", res, res->result[0], ((uint8_t *)res->result[0]) + 2, bondlen);
    hw_bluetooth_bond_data_available((res->result[0]) + 2 + namelen, bondlen);
    
    rdb_select_free_all(&head);
    rdb_close(db);
}

static void _get_bond_data_isr(const void *peer, size_t len) {
    _bondreq_peer = peer;
    _bondreq_len = len;
    _bondreq_in_progress = 0;
    DRV_LOG("btbond", APP_LOG_LEVEL_INFO, "get bond data...");
    service_submit(_get_bond_data, NULL);
}

void bluetooth_bond_acknowledge(int accepted) {
    if (!_bondreq_in_progress) {
        DRV_LOG("btbond", APP_LOG_LEVEL_ERROR, "bond acknowledge without bond in progress?");
        return;
    }
    _bondreq_in_progress = 0;
    
    if (!accepted) {
        hw_bluetooth_bond_acknowledge(0);
        return;
    }

    uint8_t *p = malloc(2 + strlen(_bondreq_name) + 1 + _bondreq_datalen);
    if (!p) {
        DRV_LOG("btbond", APP_LOG_LEVEL_ERROR, "out of memory for bond");
        hw_bluetooth_bond_acknowledge(0);
        return;
    }
    
    p[0] = strlen(_bondreq_name) + 1;
    p[1] = _bondreq_datalen;
    strcpy((char *) (p + 2), _bondreq_name);
    memcpy(p + 2 + p[0], _bondreq_data, _bondreq_datalen);
    
    struct rdb_database *db = rdb_open(RDB_ID_BLUETOOTH);
    assert(db);

    int rv = rdb_insert(db, _bondreq_peer, _bondreq_len, p, 2 + p[0] + p[1]);
    
    rdb_close(db);
    free(p);
    
    if (rv != Blob_Success) {
        const char *pv = _bondreq_peer;
        DRV_LOG("btbond", APP_LOG_LEVEL_ERROR, "failed to peer %02x%02x%02x%02x... (%d) insert into bondtab (%d)",
            pv[0], pv[1], pv[2], pv[3], _bondreq_len, rv);
        hw_bluetooth_bond_acknowledge(0);
        return;
    }
    
    DRV_LOG("btbond", APP_LOG_LEVEL_INFO, "bonded with %s", _bondreq_name);
    hw_bluetooth_bond_acknowledge(1);
}

static void _request_bond(void *p) {
    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "default bond request for peer %s, returning bonded", _bondreq_name);
    char *namedup = strdup(_bondreq_name);
    if (namedup) {
        DRV_LOG("bt", APP_LOG_LEVEL_INFO, "posting for peer %s", namedup);
        event_service_post(EventServiceCommandBluetoothPairRequest, namedup, remote_free);
    }
}

static void _request_bond_isr(const void *peer, size_t len, const char *name, const void *data, size_t datalen) {
    _bondreq_peer = peer;
    _bondreq_len = len;
    _bondreq_name = name;
    _bondreq_data = data;
    _bondreq_datalen = datalen;
    _bondreq_in_progress = 1;
    service_submit(_request_bond, NULL);
}
