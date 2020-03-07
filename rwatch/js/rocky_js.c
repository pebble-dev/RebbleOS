#include "rebbleos.h"
#include "rocky_js.h"

#if defined(REBBLE_PLATFORM_SNOWY) || defined(REBBLE_PLATFORM_SNOWY)
#include "rocky_jerry.h"
void rocky_event_loop_with_resource(uint32_t resource_id) {

}
#else
void rocky_event_loop_with_resource(uint32_t resource_id) {
    APP_LOG("rocky", APP_LOG_LEVEL_ERROR, "Attempted to execute Rocky.js with resource %u. Rocky is not available on this platform. Exiting.", resource_id);
    return;
}
#endif