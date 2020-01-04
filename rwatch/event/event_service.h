#pragma once
/* event_service.h
 * A Service bus for subscribing to messages of interest
 *   (https://github.com/pebble-dev)
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */

typedef enum EventServiceCommand {
    EventServiceCommandGeneric,
    EventServiceCommandConnectionService,
    EventServiceCommandCall,
    EventServiceCommandMusic,
    EventServiceCommandNotification,
} EventServiceCommand;


/**
 * @brief A callback prototype for an event
 */
typedef void (*EventServiceProc)(EventServiceCommand command, void *data, void *context);
typedef void (*DestroyEventProc)(void *data);

// XXX
// typedef void (*MusicServiceProc)(MusicTrackInfo *music);
// typedef void (*NotificationServiceProc)(Uuid *uuid);


/** 
 * @brief Subscribe to a given \ref EventServiceCommand
 * 
 * @param command \ref EventServiceCommand to get events for
 * @param callback callback to the function to be invoked on reception of the \ref EventServiceCommand
 */
void event_service_subscribe(EventServiceCommand command, EventServiceProc callback);

/** 
 * @brief Subscribe to a given \ref EventServiceCommand
 * 
 * @param command \ref EventServiceCommand to get events for
 * @param callback callback to the function to be invoked on reception of the \ref EventServiceCommand
 * @param context allows tagging aritary data
 */
void event_service_subscribe_with_context(EventServiceCommand command, EventServiceProc callback, void *context);

/** 
 * @brief Un-Subscribe from a given \ref EventServiceCommand
 * 
 * @param command \ref EventServiceCommand to stop watching
 */
void event_service_unsubscribe(EventServiceCommand command);

/** 
 * @brief Un-Subscribe a given thread from all watched events
 */
void event_service_unsubscribe_all(void);

/** 
 * @brief Get the context for a given \ref EventServiceCommand subscription
 * 
 * @param command \ref EventServiceCommand to get the context of
 */
void *event_service_get_context(EventServiceCommand command);

/** 
 * @brief Set the context for a given \ref EventServiceCommand subscription
 * 
 * @param command \ref EventServiceCommand to set the context of
 */
void event_service_set_context(EventServiceCommand command, void *context);

/** 
 * @brief As a client application, post a \ref EventServiceCommand data and the destroyer of said data.
 * Data is proxied through the relevant thread(s) to find a recipient of the request
 * 
 * @param command \ref EventServiceCommand to stop watching
 * @param data \ref payload data to send as part of the event. It is expected the recipient knows what to do with it
 * @param destroy_callback a callback to the method that allows the event to destroy any data it might need to
 */
void event_service_post(EventServiceCommand command, void *data, DestroyEventProc destroy_callback);

/** 
 * @brief Called from a specific thread to process the data.
 * Internal.
 * 
 * @param command \ref EventServiceCommand to trigger
 * @param data \ref payload data to send as part of the event. It is expected the recipient knows what to do with it
 * @param destroy_callback a callback to the method that allows the event to destroy any data it might need to
 */
void event_service_event_trigger(EventServiceCommand command, void *data, DestroyEventProc destroy_callback);

