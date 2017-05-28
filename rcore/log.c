/* log.c
 * routines for logging apps and system
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

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

// NOTE Probably shouldn't use from an ISR or it'll likely lock

void log_printf(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, va_list ar)
{
    char log_type[12];
    char buf[16];
    
    // XXX: this means that log_printf *must* be called first from a non-threaded context, for this check is not thread-safe!
    // XXX: verify this here.
    if (_log_mutex == NULL)
        _log_mutex = xSemaphoreCreateMutexStatic(&_log_mutex_buf);
    
    xSemaphoreTake(_log_mutex, portMAX_DELAY);

    // This is pretty cheesy. We print the sections in chunks back to back
    // This is becuase there is no %8d equiv in fmt.c so we hacky it up ourself
    switch(level)
    {
        case APP_LOG_LEVEL_ERROR:
            snprintf(log_type, sizeof(log_type), "[ERROR]");
            break;
        case APP_LOG_LEVEL_WARNING:
            snprintf(log_type, sizeof(log_type), "[WARN ]");
            break;
        case APP_LOG_LEVEL_INFO:
            snprintf(log_type, sizeof(log_type), "[INFO ]");
            break;
        case APP_LOG_LEVEL_DEBUG:
            snprintf(log_type, sizeof(log_type), "[DEBUG]");
            break;
        case APP_LOG_LEVEL_DEBUG_VERBOSE:
            snprintf(log_type, sizeof(log_type), "[DEBUV]");
            break;
        default:
            snprintf(log_type, sizeof(log_type), "[UNK  ]");
    }

    printf(log_type);
    
    _log_pad_string(layer, buf, 6);
    printf("[%s]", buf);
    
    _log_pad_string(module, buf, 6);
    printf("[%s]", buf);
    
    _log_pad_string(filename, buf, 15);
    printf("[%s", buf);
    snprintf(log_type, 5, ":%d", (int)line_no);
    
    _log_pad_string(log_type, buf, 4);
    printf("%s] ", buf);
    
    vprintf(fmt, ar);
    printf("\n");
    
    xSemaphoreGive(_log_mutex);
}


static void _log_pad_string(const char *in_str, char *padded_str, uint16_t pad_len)
{
    int len = strlen(in_str);
    padded_str[0] = 0;
    
    if (len > pad_len)
    {
        // truncate left
        snprintf(padded_str, len, in_str + len - pad_len - 1);
    }
    else
    {
        strcat(padded_str, in_str);
        // pad
        uint16_t i;
        for (i = len; i <= pad_len; i++)
        {
            padded_str[i] = ' ';
        }
        padded_str[i] = 0;
    }
}
