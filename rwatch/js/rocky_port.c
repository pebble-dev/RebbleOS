#include "rebbleos.h"
#include "rocky_port.h"

void jerry_port_fatal(jerry_fatal_code_t code)
{
    SYS_LOG("rocky", APP_LOG_LEVEL_ERROR, "VM Fatal Error Code %d", code);
}

void jerry_port_log(jerry_log_level_t level, const char *fmt, ...)
{
    LogLevel rLevel;
    switch (level)
    {
    case JERRY_LOG_LEVEL_ERROR:
        rLevel = APP_LOG_LEVEL_ERROR;
        break;
    case JERRY_LOG_LEVEL_WARNING:
        rLevel = APP_LOG_LEVEL_WARNING;
        break;
    case JERRY_LOG_LEVEL_DEBUG:
        rLevel = APP_LOG_LEVEL_DEBUG;
        break;
    case JERRY_LOG_LEVEL_TRACE:
        rLevel = APP_LOG_LEVEL_DEBUG_VERBOSE;
        break;
    default:
        rLevel = APP_LOG_LEVEL_INFO;
    }

    SYS_LOG("rocky", rLevel, "JerryScript log with level %d. NOTE: Logging unavailable", level);
}
void jerry_port_print_char(char c)
{
    SYS_LOG("rocky", APP_LOG_LEVEL_INFO, "JerryScript char: %c", c);
}

uint8_t *jerry_port_read_source(const char *file_name_p, size_t *out_size_p)
{
    return NULL;
}
void jerry_port_release_source(uint8_t *buffer_p)
{
    free(buffer_p);
}
size_t jerry_port_normalize_path(const char *in_path_p, char *out_buf_p, size_t out_buf_size, char *base_file_p)
{
    return 0;
}
jerry_value_t jerry_port_get_native_module(jerry_value_t name){
    return jerry_create_undefined();
}

double jerry_port_get_local_time_zone_adjustment(double unix_ms, bool is_utc)
{
    return 0;
}
double jerry_port_get_current_time()
{
    return 0;
}

struct jerry_context_t *jerry_port_get_current_context()
{
    return NULL;
}

void jerry_port_sleep(uint32_t sleep_time)
{
    APP_LOG("rocky", APP_LOG_LEVEL_WARNING, "Sleep not available");
}