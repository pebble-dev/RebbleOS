/* log.c
 * routines for logging apps and system
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

static SemaphoreHandle_t _log_mutex = NULL;

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

// NOTE Probably shouldn't use from an ISR or it'll likely lock

void log_printf(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    char log_type[10];
    char buf[80];
    
    if (_log_mutex == NULL)
        _log_mutex = xSemaphoreCreateMutex();
    
    xSemaphoreTake(_log_mutex, portMAX_DELAY);

    switch(level)
    {
        case APP_LOG_LEVEL_ERROR:
            snprintf(log_type, sizeof(log_type), "ERROR");
            break;
        case APP_LOG_LEVEL_WARNING:
            snprintf(log_type, sizeof(log_type), "WARNING");
            break;
        case APP_LOG_LEVEL_INFO:
            snprintf(log_type, sizeof(log_type), "INFO");
            break;
        case APP_LOG_LEVEL_DEBUG:
            snprintf(log_type, sizeof(log_type), "DEBUG");
            break;
        case APP_LOG_LEVEL_DEBUG_VERBOSE:
            snprintf(log_type, sizeof(log_type), "DEBUGV");
            break;
    }
    
    printf("[%8s][%8s][%8s] %s:%d ", log_type, layer, module, filename, (int)line_no);
    
    snprintf(buf, sizeof(buf), fmt, ar);
    printf(buf);
    printf("\n");
//     vprintf(fmt, ar); // TODO we don't have this
    va_end(ar);
    xSemaphoreGive(_log_mutex);
}
