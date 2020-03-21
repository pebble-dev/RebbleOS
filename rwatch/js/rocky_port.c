#include "jerry-api.h"
#include "jerry-port.h"
#include "rebbleos.h"
#include <stdarg.h>
#include <string.h>

void jerry_port_fatal(jerry_fatal_code_t code)
{
    SYS_LOG("rocky", APP_LOG_LEVEL_ERROR, "VM Fatal Error Code %d. Quitting.", code);
    appmanager_get_current_thread()->rocky_state.fataled = true;
    appmanager_app_quit();
}

const char *logLevelString[] = {
    [JERRY_LOG_LEVEL_ERROR] = "error",
    [JERRY_LOG_LEVEL_WARNING] = "warning",
    [JERRY_LOG_LEVEL_DEBUG] = "debug",
    [JERRY_LOG_LEVEL_TRACE] = "trace",
};

extern int vsfmt(char *buf, unsigned int len, const char *ifmt, va_list ap);

void jerry_port_log(jerry_log_level_t level, const char *fmt, ...)
{
    LogLevel appLevel;
    switch (level)
    {
    case JERRY_LOG_LEVEL_ERROR:
        appLevel = APP_LOG_LEVEL_ERROR;
        break;
    case JERRY_LOG_LEVEL_WARNING:
        appLevel = APP_LOG_LEVEL_WARNING;
        break;
    case JERRY_LOG_LEVEL_DEBUG:
        appLevel = APP_LOG_LEVEL_DEBUG;
        break;
    case JERRY_LOG_LEVEL_TRACE:
        appLevel = APP_LOG_LEVEL_DEBUG_VERBOSE;
        break;
    default:
        appLevel = APP_LOG_LEVEL_INFO;
    }

    va_list parts;
    va_start(parts, fmt);
    char outBuffer[256];
    vsfmt(outBuffer, 256, fmt, parts);
    SYS_LOG("rocky", appLevel, "js:%s> %s", logLevelString[level], outBuffer);
    va_end(parts);
}

void jerry_port_console(const char *fmt, ...)
{
    va_list parts;
    va_start(parts, fmt);
    char outBuffer[256];
    vsfmt(outBuffer, 256, fmt, parts);
    SYS_LOG("rocky", APP_LOG_LEVEL_INFO, "js> %s", outBuffer);
    va_end(parts);
}

bool jerry_port_get_time_zone(jerry_time_zone_t *tz)
{
    tz->offset = 0;
    tz->daylight_saving_time = rebble_time_get_tm()->tm_isdst;
    return true;
}

double jerry_port_get_current_time()
{
    return (double)rcore_get_time();
}