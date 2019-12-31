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

 #define BT_LOG_ENABLED
#ifdef BT_LOG_ENABLED
    #define BT_LOG SYS_LOG
#else
    #define BT_LOG NULL_LOG
#endif

/* Initialise the bluetooth module */
uint8_t bluetooth_init(void)
{
    _bt_cmd_queue = xQueueCreateStatic(_CMD_QUEUE_LENGTH, _CMD_QUEUE_SIZE, _bt_cmd_queue_buf, &_bt_cmd_queue_ptr);
    _bt_tx_mutex = xSemaphoreCreateMutexStatic(&_bt_tx_mutex_buf);
    _bt_task = xTaskCreateStatic(_bt_thread, 
                                     "BT", STACK_SZ_BT, NULL, 
                                     tskIDLE_PRIORITY + 12UL, 
                                     _bt_task_stack, &_bt_task_buf);
    
    _bt_cmd_task = xTaskCreateStatic(_bt_cmd_thread, 
                                     "BTCmd", STACK_SZ_CMD, NULL, 
                                     tskIDLE_PRIORITY + 11UL, 
                                     _bt_cmd_task_stack, &_bt_cmd_task_buf);

#ifdef BLUETOOTH_IS_BLE
    ppogatt_init();
#endif
    
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
    RebblePacketDataHeader header;
    protocol_rx_buffer_append(data, len);
    
    while (protocol_get_rx_buf_size() > 0) {
        header.data = protocol_get_rx_buffer();
        header.length = protocol_get_rx_buf_size();
    
        if (!protocol_parse_packet(&header, bluetooth_send_data))
            return; /* Packet is incomplete */

        /* seems legit. We have a valid packet. Create a data packet and process it */
        RebblePacket packet = packet_create(header.endpoint, header.length);
        assert(packet);
        uint8_t *data = packet_get_data(packet);
        memcpy(data, header.data, header.length);
        protocol_process_packet(packet);
    
        protocol_rx_buffer_consume(header.length + sizeof(RebblePacketHeader));
    }
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
    
    uint8_t noty_data[] = {/* test nofy data ripped from gb */
        0x0, 0x49, 0xb, 0xc2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0xce, 0x92, 0x83, 0xc4, 0x0, 0x0, 0x0, 0x0, 0x1e, 0xc8, 0x60, 0x5a, 0x1, 0x3, 0x1, 0x1, 0x5, 0x0, 0x54, 0x65, 0x73, 0x74, 0x74, 0x2, 0x4, 0x0, 0x54, 0x65, 0x73, 0x74, 0x3, 0x4, 0x0, 0x54, 0x65, 0x73, 0x74, 0x3, 0x4, 0x1, 0x1, 0xb, 0x0, 0x44, 0x69, 0x73, 0x6d, 0x69, 0x73, 0x73, 0x20, 0x61, 0x6c, 0x6c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    
//     bluetooth_data_rx(noty_data, 73);
//     bluetooth_send(noty_data, 73);
    
    for( ;; )
    {
        /* We are going to wait for a message. This is mostly going to be TX messages
         * We have a queue size of one, which blocks and serialises all callers
         */
        xQueueReceive(_bt_cmd_queue, &pkt, portMAX_DELAY);
        
        if (pkt.packet_type == PACKET_TYPE_TX)
        {
            BT_LOG("BT", APP_LOG_LEVEL_INFO, "TX %d byte", pkt.length);
            int n = 0;
            while(n < 5)
            {
                bt_device_request_tx(pkt.data, pkt.length);
                int rv = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
                if(rv == 0)
                    BT_LOG("BT", APP_LOG_LEVEL_ERROR, "Timed out sending!");
                
                n++;
                break;
            }
            /* XXX ugh, should be using a better dynamic pool of memory not sys malloc */
            system_free(pkt.data);
        }
    }
}

/* 
 * Utility
 */

void bluetooth_send_data(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    uint16_t ep = htons(endpoint);
    uint16_t l = htons(len);

    bluetooth_send((uint8_t *)&l, 2);
    bluetooth_send((uint8_t *)&ep, 2);
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
    uint8_t *ndata = system_malloc(len);
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
    connection_service_update(true);
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
    BT_LOG("BT", APP_LOG_LEVEL_INFO, "_tx %d", len);

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
