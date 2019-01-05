#pragma once
/* app_timer.h
 * declarations for PebbleOS AppTimer
 * libRebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stdint.h>
#include <stdbool.h>

typedef struct AppTimer AppTimer;
typedef uint16_t AppTimerHandle;

typedef void (*AppTimerCallback)(void *priv);

AppTimerHandle app_timer_register(uint32_t ms, AppTimerCallback cb, void *priv);
bool app_timer_reschedule(AppTimerHandle timer, uint32_t ms);
void app_timer_cancel(AppTimerHandle timer);
