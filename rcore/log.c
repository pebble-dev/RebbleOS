/* log.c
 * routines for logging apps and system
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "stm32_power.h"

extern int vsfmt(char *buf, unsigned int len, const char *ifmt, va_list ap);

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
    uint8_t interrupt_set = 0;
    static BaseType_t xHigherPriorityTaskWoken;
    char buf[128];
    char tbuf[16];
    
    // XXX: this means that log_printf *must* be called first from a non-threaded context, for this check is not thread-safe!
    // XXX: verify this here.
    if (_log_mutex == NULL)
        _log_mutex = xSemaphoreCreateMutexStatic(&_log_mutex_buf);
    
    if(is_interrupt_set())
    {
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreTakeFromISR(_log_mutex, &xHigherPriorityTaskWoken);
        interrupt_set = 1;
    }
    else
    {
        xSemaphoreTake(_log_mutex, portMAX_DELAY);
    }

    log_clock_enable();
 
    
#define INT_LEN 3
#define LEVEL_LEN 3
#define LAYER_LEN 8
#define MODULE_LEN 8
#define FILENM_LEN 15
#define LINENO_LEN 5
 
    snprintf(buf, INT_LEN + 1, "[%d]", interrupt_set);
    
    // This is pretty cheesy. We print the sections in chunks back to back
    // This is becuase there is no %8d equiv in fmt.c so we hacky it up ourself
    switch(level)
    {
        case APP_LOG_LEVEL_ERROR:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[E]");
            break;
        case APP_LOG_LEVEL_WARNING:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[W]");
            break;
        case APP_LOG_LEVEL_INFO:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[I]");
            break;
        case APP_LOG_LEVEL_DEBUG:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[D]");
            break;
        case APP_LOG_LEVEL_DEBUG_VERBOSE:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[V]");
            break;
        default:
            snprintf(buf + INT_LEN, LEVEL_LEN + 1, "[?]");
    }
       
    _log_pad_string(layer, tbuf, LAYER_LEN - 2);
    snprintf(buf + INT_LEN + LEVEL_LEN, LAYER_LEN + 1, "[%s]", tbuf);
    
    _log_pad_string(module, tbuf, MODULE_LEN - 2);
    snprintf(buf + INT_LEN + LEVEL_LEN + LAYER_LEN, MODULE_LEN + 1, "[%s]", tbuf);

    _log_pad_string(filename, tbuf, FILENM_LEN - 2);
    snprintf(buf + INT_LEN + LEVEL_LEN + LAYER_LEN + MODULE_LEN, FILENM_LEN + 2, "[%s", tbuf);

    snprintf(tbuf, LINENO_LEN + 2, ":%d", (int)line_no);
    _log_pad_string(tbuf, buf + INT_LEN + LEVEL_LEN + LAYER_LEN + MODULE_LEN + FILENM_LEN - 1, LINENO_LEN + 1);
    snprintf(buf + INT_LEN + LEVEL_LEN + LAYER_LEN + MODULE_LEN + FILENM_LEN + LINENO_LEN - 1, 3, "] ");

    vsfmt(buf + INT_LEN + LEVEL_LEN + LAYER_LEN + MODULE_LEN + FILENM_LEN + LINENO_LEN + 1, 128, fmt, ar);

    printf(buf);
    printf("\n");
    
    log_clock_disable();
    
    if (interrupt_set)
    {
        xSemaphoreGiveFromISR(_log_mutex, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else 
    {
        xSemaphoreGive(_log_mutex);
    }
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
