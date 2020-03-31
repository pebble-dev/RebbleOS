/* service.h
 * definitions for service worker thread
 * RebbleOS
 */

#pragma once

typedef void (*service_callback_t)(void *ctx);

void service_init();
void service_submit(service_callback_t cbk, void *ctx);
