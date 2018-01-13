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

typedef void (*AppTimerCallback)(void *priv);

AppTimer *app_timer_register(uint32_t ms, AppTimerCallback cb, void *priv);
bool app_timer_reschedule(AppTimer *timer, uint32_t ms);
void app_timer_cancel(AppTimer *timer);
