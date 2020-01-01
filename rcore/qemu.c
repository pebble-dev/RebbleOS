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

/* Configure Logging */
#define MODULE_NAME "qemu"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

extern const PebbleEndpoint qemu_endpoints[];

#define STACK_SZ_QEMU configMINIMAL_STACK_SIZE + 600

static TaskHandle_t _qemu_task;
static StackType_t _qemu_task_stack[STACK_SZ_QEMU];
static StaticTask_t _qemu_task_buf;

static void _qemu_thread(void *pvParameters);
static void _qemu_handle_packet();

static StaticSemaphore_t _qemu_mutex_mem;
static SemaphoreHandle_t _qemu_mutex;

static StaticSemaphore_t _qemu_sem_buf;
static SemaphoreHandle_t _qemu_sem;

uint8_t qemu_init(void)
{
    hw_qemu_init();
    _qemu_mutex = xSemaphoreCreateMutexStatic(&_qemu_mutex_mem);
    _qemu_task = xTaskCreateStatic(_qemu_thread,
                                   "QEMU", STACK_SZ_QEMU, NULL,
                                   tskIDLE_PRIORITY + 9UL,
                                   _qemu_task_stack, &_qemu_task_buf);
    _qemu_sem = xSemaphoreCreateBinaryStatic(&_qemu_sem_buf);

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

void qemu_rx_started_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Notify the task that the transmission is beginning. */
    xSemaphoreGiveFromISR(_qemu_sem, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void _qemu_thread(void *pvParameters)
{
    for (;;)
    {
        xSemaphoreTake(_qemu_sem, portMAX_DELAY);
        LOG_DEBUG("RX");

        uint8_t *buf = protocol_rx_buffer_request();
        size_t lenr = hw_qemu_read(buf, 255);
        protocol_rx_buffer_release(lenr);

        _qemu_handle_packet();
        protocol_rx_buffer_consume(lenr);
        hw_qemu_irq_enable();
    }
}

static void _qemu_read_header(QemuCommChannelHeader *header)
{
    uint8_t *buf = protocol_get_rx_buffer();
    QemuCommChannelHeader *raw_header = (QemuCommChannelHeader *)buf;
    header->signature = ntohs(raw_header->signature);
    header->protocol = ntohs(raw_header->protocol);
    header->len = ntohs(raw_header->len);
}

static void _qemu_handle_packet(void)
{
    QemuCommChannelHeader header;
    _qemu_read_header(&header);

    if (header.signature != QEMU_HEADER_SIGNATURE)
    {
        LOG_ERROR("Invalid header signature: %x", header.signature);
        return;
    }
    if (header.len > QEMU_MAX_DATA_LEN)
    {
        LOG_ERROR("Invalid packet size: %d", header.len);
        return;
    }

    if (protocol_get_rx_buf_size() < header.len + sizeof(QemuCommChannelHeader) + sizeof(QemuCommChannelFooter))
    {
        LOG_INFO("More Data Required %d %d", header.len, protocol_get_rx_buf_size());
        return;
    }

    EndpointHandler handler = protocol_find_endpoint_handler(header.protocol, qemu_endpoints);
    if (handler == NULL)
    {
        LOG_ERROR("Unknown protocol: %d", header.protocol);
    }

    size_t len = header.len;
    uint8_t *buf = protocol_get_rx_buffer();
    
    QemuCommChannelFooter *footer = (QemuCommChannelFooter *)(buf + header.len + sizeof(QemuCommChannelHeader));
    footer->signature = ntohs(footer->signature);
    if (footer->signature != QEMU_FOOTER_SIGNATURE)
    {
        LOG_ERROR("Invalid footer signature: %x", footer->signature);
        return;
    }

    RebblePacketDataHeader *packet = buf + sizeof(QemuCommChannelHeader);
    handler(packet);
}
