#pragma once
/* rockyjs.h
 *
 * Rocky.js (On-watch JavaScript) Implementation
 * 
 * RebbleOS
 *
 * Author: Davit Markarian <davit@udxs.me>
 */

#include <stdint.h>

typedef struct {
    int fataled;
} rocky_thread_state;


void rocky_event_loop_with_resource(uint32_t resource_id);