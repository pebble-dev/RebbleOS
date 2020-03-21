#include "rebbleos.h"
#include "rocky_js.h"

// TODO: Change the other 'defined' to match chalk, diorite
#if defined(REBBLE_PLATFORM_SNOWY) || defined(REBBLE_PLATFORM_SNOWY) || defined(REBBLE_PLATFORM_SNOWY)

#include "jerry-api.h"
#include "jcontext.h"
#include "jmem-allocator.h"

#include "rocky_lib.h"


void rocky_loop()
{
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Starting graphics...");
	Window *wnd = window_create();
	window_stack_push(wnd, true);
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Starting app loop...");
	app_event_loop();
	window_destroy(wnd);
	appmanager_app_quit();
}

void rocky_event_loop_with_resource(uint32_t resource_id)
{
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Starting Rocky.js with snapshot (resource #%u)...", resource_id);
	ResHandle snapHandle = resource_get_handle(resource_id);
	size_t snapSize = resource_size(snapHandle);
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "JS Snapshot Resource Size: %d bytes", snapSize);

	if (snapSize < 16)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "This snapshot is invalid (too small).");
		return;
	}
	if (snapSize > 128 * 1024)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "This snapshot is invalid (too large).");
		return;
	}

	uint8_t *snapBuffer = app_malloc(snapSize);
	if (snapBuffer == NULL)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Failed to allocate memory for JS snapshot resource.");
		return;
	}

	resource_load(snapHandle, snapBuffer, snapSize);
	// TODO: Fix resource_load signature
	/*
    if (resource_load(snapHandle, snapBuffer, snapSize) != snapSize)
    {
        APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Failed to load JS snapshot resource into allocated memory.");
        app_free(snapBuffer);
        return;
    }
    */

	if (memcmp("PJS", snapBuffer, 4) != 0)
	{ // Check for magic: 'PJS\0'
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Snapshot resource is not of valid format (invalid magic).");
		app_free(snapBuffer);
		return;
	}

	uint32_t snapVersion = *(snapBuffer + 4);
	if (snapVersion != 1)
	{ // Check for the only version ever made: 0x1
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Snapshot resource is not of valid format (invalid version).");
		app_free(snapBuffer);
		return;
	}

	// Skip ahead to actual JerryScript snapshot(format v6).
	uint32_t *snapshot = (uint32_t *)(snapBuffer + 8);
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "JS Snapshot Resource loaded successfully.", *snapshot);

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Creating JerryScript Context...");
	jerry_global_context = app_calloc(1, sizeof(jerry_context_t));

	if (jerry_global_context == NULL)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "JavaScript Context creation failed.");
		app_free(snapBuffer);
		return;
	}

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Allocating JerryScript Heap...");
	uint8_t *heap_loc = app_calloc(1, sizeof(jmem_heap_t) + JMEM_ALIGNMENT); // Extra space needed in case of alignment
	if (heap_loc == NULL)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Heap allocation failed.");
		app_free(snapBuffer);
		app_free(jerry_global_context);
		return;
	}
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Aligning JerryScript Heap to %d bytes from 0x%p...", JMEM_ALIGNMENT, heap_loc);
	uint8_t *aligned_loc = heap_loc;
	while ((uint32_t)aligned_loc % JMEM_ALIGNMENT != 0)
		aligned_loc += 1;
	jerry_global_heap = (jmem_heap_t *)aligned_loc;
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Aligned by %d bytes to 0x%p.", aligned_loc - heap_loc, aligned_loc);

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Allocating JerryScript Hash Table...");
	jerry_global_hash_table = app_calloc(1, sizeof(jerry_hash_table_t));
	if (jerry_global_hash_table == NULL)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Global hash table allocation failed.");
		app_free(snapBuffer);
		app_free(jerry_global_context);
		app_free(heap_loc);
		return;
	}

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Starting JavaScript...");
	jerry_init(JERRY_INIT_EMPTY);

	rocky_lib_build();

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running JavaScript Snapshot:");
	jerry_value_t out = jerry_exec_snapshot(snapshot, snapSize - 8, false);
	if (jerry_value_has_error_flag(out))
	{
		char error[128];
		if (jerry_value_is_undefined(out))
		{
			APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "UNDEF", error);
		}
		jerry_value_t msg = jerry_get_property(out, jerry_create_string("message"));
		jerry_string_to_char_buffer(msg, error, 256);
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "JavaScript program errored out: %s", error);

		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Cleaning up...");
		jerry_cleanup();
		app_free(snapBuffer);
		app_free(jerry_global_context);
		app_free(heap_loc);
		app_free(jerry_global_hash_table);
		appmanager_app_quit();
		return;
	}
	char snapOut[128];
	jerry_string_to_char_buffer(jerry_value_to_string(out), snapOut, 256);
	APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "JavaScript program result: %s", snapOut);

	if (appmanager_get_current_thread()->rocky_state.fataled)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "JavaScript Engine fatal error.");

		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Cleaning up...");
		jerry_cleanup();
		app_free(snapBuffer);
		app_free(jerry_global_context);
		app_free(heap_loc);
		app_free(jerry_global_hash_table);
		appmanager_app_quit();
		return;
	}

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Snapshot execution successful.");

	rocky_loop();

	app_free(snapBuffer);
	app_free(jerry_global_context);
	app_free(heap_loc);
	app_free(jerry_global_hash_table);
}
#else
void rocky_event_loop_with_resource(uint32_t resource_id)
{
	APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Attempted to execute Rocky.js with resource %u. Rocky is not available on this platform. Exiting.", resource_id);
}
#endif