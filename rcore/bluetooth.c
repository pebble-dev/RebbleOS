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

/* macro to swap bytes from big > little endian */
#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))

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
#define _CMD_QUEUE_LENGTH 1
#define _CMD_QUEUE_SIZE sizeof(rebble_bt_packet)
static QueueHandle_t _bt_cmd_queue;
static StaticQueue_t _bt_cmd_queue_ptr;
static uint8_t _bt_cmd_queue_buf[_CMD_QUEUE_LENGTH * _CMD_QUEUE_SIZE];

static SemaphoreHandle_t _bt_tx_mutex;
static StaticSemaphore_t _bt_tx_mutex_buf;

/* When a TX is complete, this will hold the orig task */
static TaskHandle_t _tx_task_to_notify = NULL;

static bool _enabled;
static bool _connected;


/* Packet command types */
#define PACKET_TYPE_RX 0
#define PACKET_TYPE_TX 1

static void _bt_thread(void *pvParameters);
static void _bt_cmd_thread(void *pvParameters);
static bool _parse_packet(pbl_transport_packet *pkt, uint8_t *data, size_t len);
static void _process_packet(pbl_transport_packet *pkt);
static uint8_t _bluetooth_tx(uint8_t *data, uint16_t len);

// #define BT_LOG_ENABLED
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
                                     tskIDLE_PRIORITY + 3UL, 
                                     _bt_task_stack, &_bt_task_buf);
    
    _bt_cmd_task = xTaskCreateStatic(_bt_cmd_thread, 
                                     "BTCmd", STACK_SZ_CMD, NULL, 
                                     tskIDLE_PRIORITY + 4UL, 
                                     _bt_cmd_task_stack, &_bt_cmd_task_buf);

    
    return INIT_RESP_ASYNC_WAIT;
}

void bluetooth_init_complete(uint8_t state)
{
    _enabled = (state != INIT_RESP_OK ? false : true);
    os_module_init_complete(state);
}

/*
 * Just send some raw data
 * returns bytes sent
 * DO NOT CALL FROM ISR
 */
uint32_t bluetooth_send_serial_raw(uint8_t *data, size_t len)
{
    xSemaphoreTake(_bt_tx_mutex, portMAX_DELAY);

    bt_device_request_tx(data, len);
    
    // block this thread until we are done
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(200)))
    {
        // clean unlock
        BT_LOG("BT", APP_LOG_LEVEL_DEBUG, "Sent %d bytes", len);
    }
    else
    {
        // timed out
        BT_LOG("BT", APP_LOG_LEVEL_ERROR, "Timed out sending!");
    }
    
    xSemaphoreGive(_bt_tx_mutex);
    
    return len;
}

/*
 * Callbacks and ISR from the bluetooth stack
 */


/*
 * Some data arrived from the stack
 */
void bluetooth_data_rx(uint8_t *data, size_t len)
{
    static pbl_transport_packet pkt;
    uint8_t *buf_p;  /* pointer to the message in the buffer */
    return;
    if (!_parse_packet(&pkt, data, len))
        return; // we are done, no point looking as we have no data left
    
    // seems legit
    _process_packet(&pkt);
}

/*
 * We sent a packet. it was sent.
 */
void bluetooth_tx_complete_from_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the task that the transmission is complete.
    vTaskNotifyGiveFromISR(_bt_cmd_task, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*
 * Packet processing
 */


/*
 * Parse a packet in the buffer. Will fill the given pbl_transport with
 * the parsed data
 * 
 * Returns false when done or on completion errors
 */
static bool _parse_packet(pbl_transport_packet *pkt, uint8_t *data, size_t len)
{
    uint16_t pkt_length = (data[0] << 8) | (data[1] & 0xff);
    uint16_t pkt_endpoint = (data[2] << 8) | (data[3] & 0xff);

    /* done! (usually) */
    if (pkt_length == 0)
        return false;

    /* Seems sensible */
    if (pkt_length > 2048)
    {
        BT_LOG("BT", APP_LOG_LEVEL_ERROR, "RX: payload length %d. Seems suspect!", pkt_length);
        return false;
    }   
    
    BT_LOG("BT", APP_LOG_LEVEL_INFO, "RX: GOOD packet. len %d end %d", pkt_length, pkt_endpoint);

    /* it's a valid packet. fill out passed packet and finish up */
    pkt->length = pkt_length;
    pkt->endpoint = pkt_endpoint;
    pkt->data = data + 4;
    
    BT_LOG("BT", APP_LOG_LEVEL_INFO, "RX: Done");
        
    return true;
}

/* 
 * Given a packet, process it and call the relevant function
 */
static void _process_packet(pbl_transport_packet *pkt)
{
    /*
     * We should be fast in this function!
     * Work out which message needs to be processed, and escape from here fast
     * This will likely be holding up RX otherwise.
     */
    BT_LOG("BT", APP_LOG_LEVEL_INFO, "BT Got Data L:%d", pkt->length);
        
    // Endpoint Firmware Version
    switch(pkt->endpoint) {
        case ENDPOINT_FIRMWARE_VERSION:
            process_version_packet(pkt->data);
            break;
        case ENDPOINT_SET_TIME:
            process_set_time_packet(pkt->data);
            break;
        case ENDPOINT_PHONE_MSG:
            process_notification_packet(pkt->data);
            break;
        default:
            BT_LOG("BT", APP_LOG_LEVEL_INFO, "XXX Unimplemented Endpoint %d", pkt->endpoint);
    }
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

static void _bt_cmd_thread(void *pvParameters)
{
    rebble_bt_packet pkt;
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
        if (xQueueReceive(_bt_cmd_queue, &pkt, portMAX_DELAY))
        {
            if (pkt.packet_type == PACKET_TYPE_TX)
            {
//                 BT_LOG("BT", APP_LOG_LEVEL_INFO, "TX %d byte", pkt.length);
                /* Do a blocking send. The thread will be asleep while the data is DMAed */
                bluetooth_send_serial_raw(pkt.data, pkt.length);
                
                /* Use our semaphore to notify the calling thread that we are TX done */
                if (!xTaskNotify(_tx_task_to_notify, TX_NOTIFY_COMPLETE, eSetBits))
                {
                    BT_LOG("BT", APP_LOG_LEVEL_ERROR, "TX Timed out after %dms", TX_TIMEOUT_MS);
                    xTaskNotify(_tx_task_to_notify, TX_NOTIFY_COMPLETE, eSetBits);
                    continue;
                }
                
                /* We might have been given a TX callback function to call */
//                 if (pkt.callback != NULL)
//                     pkt.callback();
            }
        }
        else
        {
            
        }
    }
}

/* 
 * Utility
 */


/*
 * Send a Pebble packet right now
 */
void bluetooth_send_packet(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    uint8_t tx_buf[len + 4];
    tx_buf[0] = ((uint8_t)(len >> 8));
    tx_buf[1] = ((uint8_t)(len & 0xff));
    tx_buf[2] = ((uint8_t)(endpoint >> 8));
    tx_buf[3] = ((uint8_t)(endpoint & 0xff));

    memcpy(&tx_buf[4], data, len);
    bluetooth_send(tx_buf, len + 4);
}

/* 
 * Request a TX through the outboud thread mechanism
 */
void bluetooth_send_async(uint8_t *data, size_t len, tx_complete_callback cb)
{
    if (!_enabled || !_connected)
        return;
    
    rebble_bt_packet packet = {
        .length = len,
        .packet_type = PACKET_TYPE_TX,
        .data = data,
        .callback = cb
    };

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
}

void bluetooth_device_disconnected(void)
{
    _connected = false;
}

static uint8_t _bluetooth_tx(uint8_t *data, uint16_t len)
{
    BaseType_t result;
    uint32_t notif_value;
    
    /* Grab the callee's task handle so we can sleep on it */
     _tx_task_to_notify = xTaskGetCurrentTaskHandle();
     
    /* This will block if there is something in progress already (queue size 1) */
    bluetooth_send_async(data, len, NULL);
    
    /* We sleep block again waiting for TX complete */
    result = xTaskNotifyWait(pdFALSE,    /* Don't clear bits on entry. */
                         0xffffffff,        /* Clear all bits on exit. */
                         &notif_value, /* Stores the notified value. */
                         pdMS_TO_TICKS(TX_TIMEOUT_MS));

    if(result == pdPASS)
    {
        if ((notif_value & TX_NOTIFY_COMPLETE) != 0)
        {
            BT_LOG("BT", APP_LOG_LEVEL_INFO, "TX Sent %d bytes", len);
        }
    }
    else
    {
        BT_LOG("BT", APP_LOG_LEVEL_ERROR, "TX Timed out after %dms", TX_TIMEOUT_MS);
        return 0;
    }


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


char *strncpy(char *a2, const char *a1, size_t len)
{
    char *origa2 = a2;
    int i = 0;
    do {
        *(a2++) = *a1;
        if (i == len)
            break;
        i++;
    } while (*(a1++));
    
    return origa2;
}

int sscanf ( const char * s, const char * format, ...)
{
    return 0;
}
