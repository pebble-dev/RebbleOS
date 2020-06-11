#include "rebbleos.h"
#include "rocky_lib.h"

#include "jcontext.h"

#define ROCKY_TYPE_SPEC_PRINT(val, type)                                         \
    if (jerry_value_is_##type((val)))                                            \
    {                                                                            \
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Variable " #val " is " #type "."); \
    }
#define ROCKY_TYPE_PRINT(val)                \
    ROCKY_TYPE_SPEC_PRINT(val, array);       \
    ROCKY_TYPE_SPEC_PRINT(val, boolean);     \
    ROCKY_TYPE_SPEC_PRINT(val, constructor); \
    ROCKY_TYPE_SPEC_PRINT(val, function);    \
    ROCKY_TYPE_SPEC_PRINT(val, number);      \
    ROCKY_TYPE_SPEC_PRINT(val, null);        \
    ROCKY_TYPE_SPEC_PRINT(val, object);      \
    ROCKY_TYPE_SPEC_PRINT(val, string);      \
    ROCKY_TYPE_SPEC_PRINT(val, undefined);

jerry_value_t rocky_lib_on(jerry_value_t func, jerry_value_t this, jerry_value_t *args, jerry_length_t count)
{
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Call to rocky.on / rocky.addEventListener with %d args.", count);

    if (count < 2)
    {
        APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Event listener argument count invalid.");
        return jerry_create_error(JERRY_ERROR_TYPE, jerry_create_string("Event Invalid"));
    }

    char type[33];
    for (int i = 0; i < 33; i++)
        type[i] = '\0';

    int typeLen = jerry_string_to_char_buffer(args[0], type, 32);
    if (typeLen == 0)
    {
        APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Event listener type invalid.");
        return jerry_create_error(JERRY_ERROR_TYPE, jerry_create_string("Event Invalid"));
    }

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Event listener for '%s' requested.", type);

    rocky_eventListener *event = rocky_eventRegister(type, args[1]);
    if (event == NULL)
    {
        APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Event listener addition failed.", type);
        return jerry_create_error(JERRY_ERROR_TYPE, jerry_create_string("Event Invalid"));
    }

    return jerry_create_undefined();
}

jerry_value_t rocky_lib_off(jerry_value_t func, jerry_value_t this, jerry_value_t *args, jerry_length_t count)
{
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Call to rocky.off / rocky.removeEventListener with %d args.", count);
}

jerry_value_t rocky_lib_postMessage(jerry_value_t func, jerry_value_t this, jerry_value_t *args, jerry_length_t count)
{
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Call to rocky.postMessage with %d args.", count);
}

jerry_value_t rocky_lib_requestDraw(jerry_value_t func, jerry_value_t this, jerry_value_t *args, jerry_length_t count)
{
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Call to rocky.requestDraw with %d args.", count);
}

void rocky_lib_addWatchInfo(jerry_value_t rocky)
{
    jerry_value_t objectName = jerry_create_string("watchInfo");
    jerry_value_t object = jerry_create_object();
    jerry_set_property(rocky, objectName, object);
    jerry_release_value(objectName);

    jerry_value_t modelName = jerry_create_string("model");
    jerry_value_t model = jerry_create_string("pebble_time_steel_black_22mm");
    jerry_set_property(object, modelName, model);
    jerry_release_value(modelName);
    jerry_release_value(model);

    jerry_value_t platformName = jerry_create_string("platform");
    jerry_value_t platform = jerry_create_string("basalt"); // TODO: The other platforms (chalk, diorite)
    jerry_set_property(object, platformName, platform);
    jerry_release_value(platformName);
    jerry_release_value(platform);

    jerry_value_t languageName = jerry_create_string("language");
    jerry_value_t language = jerry_create_string("en-US"); // TODO: Actually implement this, unlike Pebble
    jerry_set_property(object, languageName, language);
    jerry_release_value(languageName);
    jerry_release_value(language);

    //jerry_value_t firmwareName = jerry_create_string("firmware");
    //jerry_value_t firmwareObject = jerry_create_object();

    //jerry_set_property(object, platformName, platform);
    //jerry_release_value(platformName);
    //jerry_release_value(platform);

    jerry_release_value(object);
}

void rocky_lib_build()
{
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Creating Rocky Library...");
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t libName = jerry_create_string((const jerry_char_t *)"_rocky");
    jerry_value_t libObject = jerry_create_object();

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Committing to JavaScript Global Environment...");
    jerry_set_property(global, libName, libObject);
    jerry_release_value(global);
    jerry_release_value(libName);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding function 'rocky.on'...");
    jerry_value_t libOnName = jerry_create_string((const jerry_char_t *)"on");
    jerry_value_t libOnFunc = jerry_create_external_function(rocky_lib_on);
    jerry_set_property(libObject, libOnName, libOnFunc);
    jerry_release_value(libOnName);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding alias 'rocky.addEventListener'...");
    jerry_value_t libAddEventListenerName = jerry_create_string((const jerry_char_t *)"addEventListener");
    jerry_set_property(libObject, libAddEventListenerName, libOnFunc);
    jerry_release_value(libAddEventListenerName);
    jerry_release_value(libOnFunc);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding function 'rocky.off'...");
    jerry_value_t libOffName = jerry_create_string((const jerry_char_t *)"off");
    jerry_value_t libOffFunc = jerry_create_external_function(rocky_lib_off);
    jerry_set_property(libObject, libOffName, libOffFunc);
    jerry_release_value(libOffName);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding alias 'rocky.removeEventListener'...");
    jerry_value_t libRemoveEventListenerName = jerry_create_string((const jerry_char_t *)"removeEventListener");
    jerry_set_property(libObject, libRemoveEventListenerName, libOffFunc);
    jerry_release_value(libRemoveEventListenerName);
    jerry_release_value(libOffFunc);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding function 'rocky.postMessage'...");
    jerry_value_t libPostMessageName = jerry_create_string((const jerry_char_t *)"postMessage");
    jerry_value_t libPostMessageFunc = jerry_create_external_function(rocky_lib_postMessage);
    jerry_set_property(libObject, libPostMessageName, libPostMessageFunc);
    jerry_release_value(libPostMessageName);
    jerry_release_value(libPostMessageFunc);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding function 'rocky.requestDraw'...");
    jerry_value_t libRequestDrawName = jerry_create_string((const jerry_char_t *)"requestDraw");
    jerry_value_t libRequestDrawFunc = jerry_create_external_function(rocky_lib_requestDraw);
    jerry_set_property(libObject, libRequestDrawName, libRequestDrawFunc);
    jerry_release_value(libRequestDrawName);
    jerry_release_value(libRequestDrawFunc);

    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Adding object 'rocky.watchInfo'...");
    rocky_lib_addWatchInfo(libObject);

    //TODO: userPreferences

    jerry_release_value(libObject);
    APP_LOG("rocky", APP_LOG_LEVEL_INFO, "Done building Rocky library.");
}
