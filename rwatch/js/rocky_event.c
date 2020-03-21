#include "rocky_lib.h"
#include "connection_service.h"
#include <string.h>

const char *const rocky_eventListenerTypeStrings[] = {
    "UNKNOWN",
    "draw",
    "secondchange",
    "minutechange",
    "hourchange",
    "daychange",
    "memorypressure",
    "message",
    "postmessageconnected",
    "postmessagedisconnected",
    "postmessageerror"

};

static list_head rocky_eventList = LIST_HEAD(rocky_eventList);

#define ROCKY_EVENT_TYPES_COUNT (sizeof(rocky_eventListenerTypeStrings) / sizeof(char *))

int rocky_eventList_count(rocky_eventListenerType type)
{
    int count = 0;
    rocky_eventListener *listener;
    list_foreach(listener, &rocky_eventList, rocky_eventListener, node)
    {
        if (listener->type == type)
        {
            count++;
        }
    }

    return count;
}

void rocky_eventHandleConnection(bool connected)
{
    if (connected)
        rocky_eventHandle(rocky_eventListenerType_postMessageConnected);
    else
        rocky_eventHandle(rocky_eventListenerType_postMessageDisconnected);
}

void rocky_eventManageConnections(rocky_eventListener *event)
{
    if (rocky_eventList_count(rocky_eventListenerType_postMessageConnected) + rocky_eventList_count(rocky_eventListenerType_postMessageDisconnected) == 1)
    {
        connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = NULL, .pebblekit_connection_handler = rocky_eventHandleConnection});
        bool connected = connection_service_peek_pebblekit_connection();
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Subscribing to Connection Service. Running Callback...");
        if(connected && event->type == rocky_eventListenerType_postMessageConnected) jerry_run(event->callback);
        else if(!connected && event->type == rocky_eventListenerType_postMessageDisconnected) jerry_run(event->callback);
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "'%s' callback executed immediately.", rocky_eventListenerTypeStrings[event->type]);
    }
    else if (rocky_eventList_count(rocky_eventListenerType_postMessageConnected) + rocky_eventList_count(rocky_eventListenerType_postMessageDisconnected) == 0)
    {
        connection_service_unsubscribe();
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Unsubscribed from Connection Service.");
    }
}

rocky_eventListener *rocky_eventRegister(char *type, jerry_value_t callback)
{
    rocky_eventListener *event = app_malloc(sizeof(rocky_eventListener));
    if (event == NULL)
    {
        return NULL;
    }

    event->type = rocky_eventListenerType_UNKNOWN;

    for (int i = 1; i < ROCKY_EVENT_TYPES_COUNT; i++)
    {
        if (strcmp(type, rocky_eventListenerTypeStrings[i]) == 0)
        {
            event->type = i;
            APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Event listener type matched as rocky_eventListenerType[%d].", i);
        }
    }

    if (type == rocky_eventListenerType_UNKNOWN)
    {
        APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Event listener type unknown.");
        app_free(event);
        return NULL;
    }

    if (!jerry_value_is_function(callback))
    {
        APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Event listener callback not a function.");
        app_free(event);
        return NULL;
    }

    jerry_acquire_value(callback);
    event->callback = callback;

    list_insert_tail(&rocky_eventList, &event->node);
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Event listener added to list.");

    switch (event->type)
    {
    case rocky_eventListenerType_postMessageConnected:
    case rocky_eventListenerType_postMessageDisconnected:
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running message connection callback setup...");
        rocky_eventManageConnections(event);
    case rocky_eventListenerType_secondChange:
    case rocky_eventListenerType_minuteChange:
    case rocky_eventListenerType_hourChange:
    case rocky_eventListenerType_dayChange:
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running time change callback setup...");
        break;
    default:
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "No additional work required.");
        break;
    }

    return event;
}

void rocky_eventHandle(rocky_eventListenerType type)
{
}