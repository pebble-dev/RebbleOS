#pragma once
/* log.h
 * routines for logging apps and system
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdarg.h>

#define NULL_LOG(module_, lvl_, fmt_, ...) {;}
#define SYS_LOG(module_, lvl_, fmt_, ...) \
            log_printf_to_ar(module_, "SYS", lvl_, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)
#define KERN_LOG(module_, lvl_, fmt_, ...) \
            log_printf_to_ar(module_, "KERN", lvl_, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)
#define DRV_LOG(module_, lvl_, fmt_, ...) \
            log_printf_to_ar(module_, "DRIVER", lvl_, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)
#define APP_LOG(module_, lvl_, fmt_, ...) \
            log_printf_to_ar(module_, "APP", lvl_, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)


#define _LOG_NONE  0
#define _LOG_DEBUG 1
#define _LOG_INFO  2
#define _LOG_WARN  4
#define _LOG_ERROR 8

#define RBL_LOG_LEVEL_ALL (_LOG_DEBUG | _LOG_INFO | _LOG_ERROR)
#define RBL_LOG_LEVEL_DEBUG (_LOG_DEBUG | _LOG_INFO | _LOG_ERROR)
#define RBL_LOG_LEVEL_INFO (_LOG_INFO | _LOG_ERROR)
#define RBL_LOG_LEVEL_WARN (_LOG_WARN | _LOG_ERROR)
#define RBL_LOG_LEVEL_ERROR _LOG_ERROR
#define RBL_LOG_LEVEL_NONE _LOG_NONE

#define __LOG_AR(name_, type_, log_type_, fmt_, ...) \
        log_printf_to_ar(name_, type_, log_type_, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)

#define LOG_INFO(fmt_, ...) \
        if (LOG_LEVEL & _LOG_INFO) \
            __LOG_AR(MODULE_NAME, MODULE_TYPE, APP_LOG_LEVEL_INFO, fmt_, ##__VA_ARGS__)
#define LOG_DEBUG(fmt_, ...) \
        if (LOG_LEVEL & _LOG_DEBUG) \
            __LOG_AR(MODULE_NAME, MODULE_TYPE, APP_LOG_LEVEL_DEBUG, fmt_, ##__VA_ARGS__)
#define LOG_WARN(fmt_, ...) \
        if (LOG_LEVEL & _LOG_WARN) \
            __LOG_AR(MODULE_NAME, MODULE_TYPE, APP_LOG_LEVEL_WARNING, fmt_, ##__VA_ARGS__)
#define LOG_ERROR(fmt_, ...) \
        if (LOG_LEVEL & _LOG_ERROR) \
            __LOG_AR(MODULE_NAME, MODULE_TYPE, APP_LOG_LEVEL_ERROR, fmt_, ##__VA_ARGS__)

typedef enum LogLevel {
    APP_LOG_LEVEL_ERROR,
    APP_LOG_LEVEL_WARNING,
    APP_LOG_LEVEL_INFO,
    APP_LOG_LEVEL_DEBUG,
    APP_LOG_LEVEL_DEBUG_VERBOSE
} LogLevel;

void log_init();

void app_log_trace(uint8_t level, const char *filename, uint32_t f, const char *fmt, ...);

void log_printf(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, va_list ar);
void log_printf_to_ar(const char *layer, const char *module, uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...);
