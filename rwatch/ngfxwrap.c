/* ngfxwrap.c
 * Shim between RebbleOS and Neographics
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "main.h"
#include "context.h"
#include "appmanager.h"
#include "display.h"
#include "overlay_manager.h"

void rwatch_neographics_init(app_running_thread *thread)
{
    thread->graphics_context = n_root_graphics_context_from_buffer(display_get_buffer());
}

n_GContext *rwatch_neographics_get_global_context(void)
{
    app_running_thread *_this_thread = appmanager_get_current_thread();
    
    if (_this_thread->thread_type == AppThreadOverlay)
    {
        OverlayWindow *owindow = overlay_stack_get_top_overlay_window();
        return owindow->graphics_context;
    }
    return _this_thread->graphics_context;
}
