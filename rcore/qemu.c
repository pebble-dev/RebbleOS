/* qemu.c
 * A thread for qemu packets to be received and relayed into protocol handlers
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "qemu.h"
#include "hw_qemu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "rebbleos.h"
#include "endpoint.h"
#include "protocol.h"
#include "protocol_service.h"

/* Configure Logging */
#define MODULE_NAME "qemu"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

extern const PebbleEndpoint qemu_endpoints[];

#define STACK_SZ_QEMU configMINIMAL_STACK_SIZE + 400

static TaskHandle_t _qemu_task;
static StackType_t _qemu_task_stack[STACK_SZ_QEMU];
static StaticTask_t _qemu_task_buf;

static void _qemu_thread(void *pvParameters);
static int _qemu_handle_packet();
static bool _qemu_process_all_messages(void);

static StaticSemaphore_t _qemu_mutex_mem;
static SemaphoreHandle_t _qemu_mutex;

static StaticSemaphore_t _qemu_sem_buf;
static SemaphoreHandle_t _qemu_sem;

static int is_qemu = 0;

uint8_t qemu_init(void)
{
    is_qemu = 1;
    
    hw_qemu_init();
    _qemu_mutex = xSemaphoreCreateMutexStatic(&_qemu_mutex_mem);
    _qemu_sem = xSemaphoreCreateBinaryStatic(&_qemu_sem_buf);
    _qemu_task = xTaskCreateStatic(_qemu_thread,
                                   "QEMU", STACK_SZ_QEMU, NULL,
                                   tskIDLE_PRIORITY + 5UL,
                                   _qemu_task_stack, &_qemu_task_buf);

    return INIT_RESP_OK;
}

size_t qemu_read(void *buffer, size_t max_len)
{
    xSemaphoreTake(_qemu_mutex, portMAX_DELAY);
    size_t bytes_read = hw_qemu_read(buffer, max_len);
    xSemaphoreGive(_qemu_mutex);
    return bytes_read;
}


size_t qemu_write(const void *buffer, size_t len)
{
    xSemaphoreTake(_qemu_mutex, portMAX_DELAY);
    size_t bytes_written = hw_qemu_write(buffer, len);
    xSemaphoreGive(_qemu_mutex);

    return bytes_written;
}

void qemu_send_data(uint16_t endpoint, uint8_t *data, uint16_t len)
{
    if (!is_qemu)
        return;
    
    xSemaphoreTake(_qemu_mutex, portMAX_DELAY);
    QemuCommChannelHeader header;
    header.signature = htons(QEMU_HEADER_SIGNATURE);
    header.protocol = htons(QemuProtocol_SPP);
    header.len = htons(len + 4);
    hw_qemu_write((const void *)&header, sizeof(QemuCommChannelHeader));

    /* Write the length out */
    uint16_t l = htons(len);
    hw_qemu_write((const void *)&l, 2);

    /* Write the endpoint out */
    uint16_t ep = htons(endpoint);
    hw_qemu_write((const void *)&ep, 2);

    /* data */
    hw_qemu_write((const void *)data, len);

    /* footer */
    QemuCommChannelFooter footer = {
            .signature = htons(QEMU_FOOTER_SIGNATURE)
        };
    hw_qemu_write((const void *)&footer, sizeof(QemuCommChannelFooter));

    xSemaphoreGive(_qemu_mutex);
}

#ifdef REBBLEOS_TESTING
void qemu_reply_test(uint8_t *data, uint16_t len)
{
    xSemaphoreTake(_qemu_mutex, portMAX_DELAY);
    QemuCommChannelHeader header;
    header.signature = htons(QEMU_HEADER_SIGNATURE);
    header.protocol = htons(QemuProtocol_Tests);
    header.len = htons(len);
    hw_qemu_write((const void *)&header, sizeof(QemuCommChannelHeader));

    /* data */
    hw_qemu_write((const void *)data, len);

    /* footer */
    QemuCommChannelFooter footer = {
            .signature = htons(QEMU_FOOTER_SIGNATURE)
        };
    hw_qemu_write((const void *)&footer, sizeof(QemuCommChannelFooter));

    xSemaphoreGive(_qemu_mutex);
}
#endif

void qemu_rx_started_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Notify the task that the transmission is beginning. */
    xSemaphoreGiveFromISR(_qemu_sem, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void _qemu_thread(void *pvParameters)
{
    mem_thread_set_heap(&mem_heaps[HEAP_LOWPRIO]);
    #define READ_SIZE 64
    static uint8_t buf[READ_SIZE];

    for (;;)
    {
        xSemaphoreTake(_qemu_sem, portMAX_DELAY);
        bool done = false;
        int invalid_count = 0;
        
        while(!done)
        {
            size_t lenr = hw_qemu_read(buf, READ_SIZE);

            if (!lenr) {
                break;
            }

            while(1) {
                int rv = _qemu_handle_packet(buf, &lenr);
                if (rv == PACKET_PROCESSED) { /* All data done */
                    break;
                } else if (rv == PACKET_MORE_DATA_REQD) {
                    break;
                } else if (rv == PACKET_INVALID) {
                    LOG_ERROR("TI");
                    if (invalid_count == 3) {
                        protocol_rx_buffer_reset();
                        done = true;
                    }
                    invalid_count++;
                    break;
                }
            }
        }
        hw_qemu_irq_enable();
    }
}

static void _qemu_read_header(QemuCommChannelHeader *header)
{
    //QemuCommChannelHeader *raw_header = (QemuCommChannelHeader *)buf;
    header->signature = ntohs(header->signature);
    header->protocol = ntohs(header->protocol);
    header->len = ntohs(header->len);
}

static int _qemu_handle_packet(uint8_t *buf, size_t *len)
{
    static QemuCommChannelHeader header;
    static QemuCommChannelHeader footer;
    static int idx = 0;

    size_t processed = 0;
    size_t sz = 0;
    int buflen = *len;
    int rv = 0;

    if (idx < sizeof(QemuCommChannelHeader)) {
        sz = buflen >= sizeof(QemuCommChannelHeader) 
                ? sizeof(QemuCommChannelHeader) 
                : buflen;
        memcpy(((void *)(&header)) + idx, buf, sz);
        idx += sz;
        processed += sz;

        if (idx < sizeof(QemuCommChannelHeader))
            return PACKET_MORE_DATA_REQD;

        _qemu_read_header(&header);

        if (header.signature != QEMU_HEADER_SIGNATURE) {
            LOG_ERROR("Invalid header signature: %x", header.signature);
            return PACKET_INVALID;
        }

        if (header.len > QEMU_MAX_DATA_LEN) {
            LOG_ERROR("Invalid packet size: %d", header.len);
            rv = PACKET_INVALID;
            goto error;
        }
    }

    /* So far so good, we have at least a valid looking header */
    if (buflen - processed == 0) {
        return PACKET_MORE_DATA_REQD;
    }
    int total_len = header.len + sizeof(QemuCommChannelHeader);

    if (idx < total_len) {
        sz = total_len - idx > buflen - processed 
                ? buflen - processed 
                : total_len - idx;
        /* start to chunk the data into the rx buffer */
        if (protocol_rx_buffer_append(buf + processed, sz) != PROTOCOL_BUFFER_OK) {
            rv = PACKET_INVALID;
            goto error;
        }
        idx += sz;
        processed += sz;
        *len = buflen - processed;
    }

    if (idx < total_len)
        return PACKET_MORE_DATA_REQD;

    /* We have the data now, just waiting for the footer */
    sz = sizeof(QemuCommChannelFooter) > buflen - processed
                ? buflen - processed
                : sizeof(QemuCommChannelFooter);
    memcpy(((void *)(&footer)) + idx - total_len, buf + processed, sz);
    idx += sz;
    processed += sz;

    /* is there enough data for a footer? */
    if (idx < total_len + sizeof(QemuCommChannelFooter))
        return PACKET_MORE_DATA_REQD;

    if (ntohs(footer.signature) != QEMU_FOOTER_SIGNATURE) {
        LOG_ERROR("Invalid footer signature: %x", ntohs(footer.signature));
        rv = PACKET_INVALID;
        goto error;
    }

    /* we have a valid header, payload in the rx buffer and a footer */
    EndpointHandler handler = protocol_find_endpoint_handler(header.protocol, qemu_endpoints);
    if (handler == NULL) {
        LOG_ERROR("Unknown protocol: %d", header.protocol);
        rv = PACKET_INVALID;
        goto error;
    }

    RebblePacket p = packet_create_with_data(0, NULL, header.len);
    packet_set_transport(p, qemu_send_data);
    handler(p);

    if (buflen - processed > 0) {
        memmove(buf, buf + processed, buflen - processed);
        *len = buflen - processed;
        return PACKET_BUFFER_HAS_DATA; /* more work to do */
    }
    
    /* we are done */
    rv = PACKET_PROCESSED;

error:
    idx = 0;
    return rv;
}
