#include "rocky_lib.h"
#include "connection_service.h"
#include "rebble_time.h"
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

// TODO: Move to thread state
static list_head rocky_eventList = LIST_HEAD(rocky_eventList);

#define ROCKY_EVENT_TYPES_COUNT (sizeof(rocky_eventListenerTypeStrings) / sizeof(char *))

int rocky_eventList_count(rocky_eventListenerType type)
{
	int count = 0;
	rocky_eventListener *listener;
	list_foreach(listener, &rocky_eventList, rocky_eventListener, node)
	{
		if (listener->type == type)
			count++;
	}

	return count;
}

void rocky_eventHandleConnection(bool connected)
{
	return;
	jerry_value_t undef = jerry_create_undefined();
	if (connected)
		rocky_eventHandle(rocky_eventListenerType_postMessageConnected, undef);
	else
		rocky_eventHandle(rocky_eventListenerType_postMessageDisconnected, undef);
	jerry_release_value(undef);
}

void rocky_eventHandleTickTimer(struct tm *tick_time, TimeUnits units_changed)
{
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Tick Timer event fired with flags 0x%x", units_changed);

	// TODO: better Date assembly
	/*
	jerry_value_t global = jerry_get_global_object();
	jerry_value_t jsDateName = jerry_create_string("Date");
	jerry_value_t jsDate = jerry_get_property(global, jsDateName);
	jerry_release_value(jsDateName);
	jerry_release_value(global);

	if (jerry_value_is_undefined(jsDate))
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Date bad!");

	jerry_value_t time = jerry_create_object();
	jerry_release_value(jsDate); */

	jerry_value_t time = jerry_create_undefined();
	if ((units_changed & SECOND_UNIT) != 0)
		rocky_eventHandle(rocky_eventListenerType_secondChange, time);
	if ((units_changed & MINUTE_UNIT) != 0)
		rocky_eventHandle(rocky_eventListenerType_minuteChange, time);
	if ((units_changed & HOUR_UNIT) != 0)
		rocky_eventHandle(rocky_eventListenerType_hourChange, time);
	if ((units_changed & DAY_UNIT) != 0)
		rocky_eventHandle(rocky_eventListenerType_dayChange, time);

	jerry_release_value(time);
}

void rocky_eventManageConnections(rocky_eventListener *event)
{
	if (rocky_eventList_count(rocky_eventListenerType_postMessageConnected) + rocky_eventList_count(rocky_eventListenerType_postMessageDisconnected) == 1)
	{
		connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = NULL, .pebblekit_connection_handler = rocky_eventHandleConnection});
		bool connected = connection_service_peek_pebblekit_connection();
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Subscribing to Connection Service. Running Callback...");
		if (connected && event->type == rocky_eventListenerType_postMessageConnected)
			jerry_run(event->callback);
		else if (!connected && event->type == rocky_eventListenerType_postMessageDisconnected)
			jerry_run(event->callback);
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "'%s' callback executed immediately.", rocky_eventListenerTypeStrings[event->type]);
	}
	else if (rocky_eventList_count(rocky_eventListenerType_postMessageConnected) + rocky_eventList_count(rocky_eventListenerType_postMessageDisconnected) == 0)
	{
		connection_service_unsubscribe();
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Unsubscribed from Connection Service.");
	}
}

void rocky_eventManageTickTimer(rocky_eventListener *event)
{
	TimeUnits units = 0;
	if (rocky_eventList_count(rocky_eventListenerType_secondChange) != 0)
		units |= SECOND_UNIT;
	if (rocky_eventList_count(rocky_eventListenerType_minuteChange) != 0)
		units |= MINUTE_UNIT;
	if (rocky_eventList_count(rocky_eventListenerType_hourChange) != 0)
		units |= HOUR_UNIT;
	if (rocky_eventList_count(rocky_eventListenerType_dayChange) != 0)
		units |= DAY_UNIT;

	if (units != 0)
	{
		tick_timer_service_subscribe(units, rocky_eventHandleTickTimer);
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Subscribed to Tick Timer Service with flags 0x%x.", units);
	}
	else
	{
		tick_timer_service_unsubscribe();
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Unsubscribed from Tick Timer Service.");
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

	if (event->type == rocky_eventListenerType_UNKNOWN)
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

	event->callback = jerry_acquire_value(callback);

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
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running tick timer callback setup...");
		rocky_eventManageTickTimer(event);
		break;
	default:
		APP_LOG("rocky", APP_LOG_LEVEL_INFO, "No additional work required.");
		break;
	}

	return event;
}

void rocky_eventHandle(rocky_eventListenerType type, jerry_value_t arg)
{
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running handlers for event '%s'...", rocky_eventListenerTypeStrings[type]);

	rocky_eventListener *listener;
	int count = 0;
	list_foreach(listener, &rocky_eventList, rocky_eventListener, node)
	{
		if (listener->type == type)
		{
			count++;
			APP_LOG("rocky", APP_LOG_LEVEL_INFO, "  - Handler 0x%x", (uint32_t)listener->callback);

			jerry_value_t jsThis = jerry_create_undefined();
			jerry_value_t args[] = {arg};
			jerry_call_function(listener->callback, jsThis, args, 1);
			//jerry_release_value(jsThis);
		}
	}
	//jerry_cleanup();
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "%d handler(s) for '%s' executed.", count, rocky_eventListenerTypeStrings[type]);
}