#pragma once
/* rocky_lib.h
 *
 * Rocky.js (On-watch JavaScript) Implementation
 * Rocky API Library
 * RebbleOS
 *
 * Author: Davit Markarian <davit@udxs.me>
 */

#include "jerry-api.h"
#include "node_list.h"
#include "rocky_canvas.h"

typedef enum
{
    rocky_eventListenerType_UNKNOWN,
    rocky_eventListenerType_draw,
    rocky_eventListenerType_secondChange,
    rocky_eventListenerType_minuteChange,
    rocky_eventListenerType_hourChange,
    rocky_eventListenerType_dayChange,
    rocky_eventListenerType_memoryPressure,
    rocky_eventListenerType_message,
    rocky_eventListenerType_postMessageConnected,
    rocky_eventListenerType_postMessageDisconnected,
    rocky_eventListenerType_postMessageError,

} rocky_eventListenerType;

extern const char* const rocky_eventListenerTypeStrings[];

typedef struct
{
    rocky_eventListenerType type;
    jerry_value_t callback;
    list_node node;
} rocky_eventListener;

rocky_eventListener *rocky_eventRegister(char *type, jerry_value_t callback);
rocky_eventListener *rocky_eventRemove(char *type, jerry_value_t callback);
void rocky_eventHandle(rocky_eventListenerType type, jerry_value_t arg);

void rocky_lib_build();