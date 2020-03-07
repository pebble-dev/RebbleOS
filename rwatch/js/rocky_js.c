#include "rebbleos.h"
#include "rocky_js.h"

#if defined(REBBLE_PLATFORM_SNOWY) || defined(REBBLE_PLATFORM_SNOWY)

#include "jerryscript.h"

void *rocky_jerry_ctx_alloc(size_t size, void *userdata)
{
	return app_malloc(size);
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

	uint32_t *snapshot = snapBuffer + 8;
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "JS Snapshot Resource loaded successfully.");

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Creating JavaScript Context...");

	app_running_thread *this_thread = appmanager_get_current_thread();

	this_thread->js_context = jerry_create_context(64 * 1024, rocky_jerry_ctx_alloc, NULL);
	if (this_thread->js_context == NULL)
	{
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "JavaScript Context creation failed.");
		app_free(snapBuffer);
		return;
	}

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Starting JavaScript...");
	jerry_init(JERRY_INIT_EMPTY);

	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Running JavaScript Snapshot...");
	jerry_value_t error = jerry_get_error_type(jerry_exec_snapshot(snapshot, snapSize - 8, 0, 0));
	if(error != JERRY_ERROR_NONE){
		APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Program execution failed. JavaScript engine reported error type %d.", error);
		app_free(snapBuffer);
		app_free(this_thread->js_context);
		return;
	}
	APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Snapshot execution successful.");

}
#else
void rocky_event_loop_with_resource(uint32_t resource_id)
{
	APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Attempted to execute Rocky.js with resource %u. Rocky is not available on this platform. Exiting.", resource_id);
}
#endif