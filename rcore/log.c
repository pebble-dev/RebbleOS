/* log.c
 * routines for logging apps and system
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "minilib.h"
#include "rtoswrap.h"

QUEUE_DEFINE(log, char, 1536);
static volatile int _log_isrunning = 0;
static void _log_entry(void *arg) {
    QUEUE_CREATE(log);
    _log_isrunning = 1;
    while (1) {
        char c;
        xQueueReceive(QUEUE_HANDLE(log), &c, portMAX_DELAY);
        log_clock_enable();
        printf("%c", c);
        log_clock_disable();
    }
}
THREAD_DEFINE(log, 300, tskIDLE_PRIORITY + 10UL, _log_entry);

static void _logc(void *priv, char c) {
    if (!_log_isrunning) {
        log_clock_enable();
        printf("%c", c);
        log_clock_disable();
        return;
    }
    BaseType_t woken = pdFALSE;
    
    if (xPortIsInsideInterrupt()) {
        xQueueSendFromISR(QUEUE_HANDLE(log), &c, &woken);
        portYIELD_FROM_ISR(woken);
    } else {
        xQueueSend(QUEUE_HANDLE(log), &c, portMAX_DELAY);
    }
}

void log_init() {
    QUEUE_CREATE(log);
    THREAD_CREATE(log);
}

static SemaphoreHandle_t _log_mutex = NULL;
static StaticSemaphore_t _log_mutex_buf;
static void _log_pad_string(const char *in_str, char *padded_str, uint16_t pad_len);

/*
 * Print log output (like APP_LOG:   INFO filename.c message)
 */
void app_log_trace(uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    log_printf("APP", "APP", level, filename, line_no, fmt, ar);
    va_end(ar);
}

void log_printf_to_ar(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    log_printf(layer, module, level, filename, line_no, fmt, ar);
    va_end(ar);
}


static void _vlogf(const char *s, va_list args) {
    struct fmtctx ctx = {
        .str = s,
        .out = _logc,
    };
    fmt(&ctx, args);
}

static void _logf(const char *s, ...) {
    va_list args;
    va_start(args, s);
    _vlogf(s, args);
    va_end(args);
}

void log_printf(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, va_list ar)
{
    uint8_t interrupt_set = 0;
    static BaseType_t xHigherPriorityTaskWoken;
    char tbuf[16];
    
    // XXX: this means that log_printf *must* be called first from a non-threaded context, for this check is not thread-safe!
    if (_log_mutex == NULL)
        _log_mutex = xSemaphoreCreateMutexStatic(&_log_mutex_buf);
    
    /* Opportunistically avoid smashing two things together.  Interrupts can
     * smash, since they can't block.  */
    if(!xPortIsInsideInterrupt())
        xSemaphoreTake(_log_mutex, portMAX_DELAY);

#define INT_LEN 6
#define LEVEL_LEN 3
#define LAYER_LEN 8
#define MODULE_LEN 8
#define FILENM_LEN 15
#define LINENO_LEN 5
 
    _logf(xPortIsInsideInterrupt() ? "[ISR]" : "[THD]");
    app_running_thread *thread = appmanager_get_current_thread();
    _logf(thread ? "[%d]" : "[x]", thread ? thread->thread_type : -1);
    
    // This is pretty cheesy. We print the sections in chunks back to back
    // This is becuase there is no %8d equiv in fmt.c so we hacky it up ourself
    switch(level)
    {
        case APP_LOG_LEVEL_ERROR:
            _logf("[E]");
            break;
        case APP_LOG_LEVEL_WARNING:
            _logf("[W]");
            break;
        case APP_LOG_LEVEL_INFO:
            _logf("[I]");
            break;
        case APP_LOG_LEVEL_DEBUG:
            _logf("[D]");
            break;
        case APP_LOG_LEVEL_DEBUG_VERBOSE:
            _logf("[V]");
            break;
        default:
            _logf("[?]");
    }
       
    _log_pad_string(layer, tbuf, LAYER_LEN - 2);
    _logf("[%s]", tbuf);
    
    _log_pad_string(module, tbuf, MODULE_LEN - 2);
    _logf("[%s]", tbuf);

    _log_pad_string(filename, tbuf, FILENM_LEN - 2);
    _logf("[%s:% 4d] ", tbuf, (int)line_no);

    _vlogf(fmt, ar);
    _logf("\n");
    
    if (!xPortIsInsideInterrupt())
        xSemaphoreGive(_log_mutex);
}


static void _log_pad_string(const char *in_str, char *padded_str, uint16_t pad_len)
{
    int len = strlen(in_str);
    padded_str[0] = 0;
    
    if (len > pad_len)
    {
        // truncate left
        snprintf(padded_str, len, in_str + len - pad_len);
    }
    else
    {
        strcat(padded_str, in_str);
        // pad
        uint16_t i;
        for (i = len; i < pad_len; i++)
        {
            padded_str[i] = ' ';
        }
        padded_str[i] = 0;
    }
}
