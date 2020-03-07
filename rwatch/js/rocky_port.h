#include "jerryscript-port.h"

void jerry_port_fatal(jerry_fatal_code_t code);

void jerry_port_log(jerry_log_level_t level, const char *fmt, ...);
void jerry_port_print_char(char c);

uint8_t *jerry_port_read_source(const char *file_name_p, size_t *out_size_p);
void jerry_port_release_source(uint8_t *buffer_p);
size_t jerry_port_normalize_path(const char *in_path_p, char *out_buf_p, size_t out_buf_size, char *base_file_p);
jerry_value_t jerry_port_get_native_module(jerry_value_t name);

double jerry_port_get_local_time_zone_adjustment(double unix_ms, bool is_utc);
double jerry_port_get_current_time();

struct jerry_context_t *jerry_port_get_current_context();

void jerry_port_sleep(uint32_t sleep_time);
