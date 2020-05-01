#pragma once
/* tick_timer_service.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler);
void tick_timer_service_unsubscribe(void);
void tick_timer_service_unsubscribe_thread(app_running_thread *thread);