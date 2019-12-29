/* call_overlay.c
 * Draws a small call notification window
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "platform_res.h"
#include "notification_manager.h"
#include "protocol.h"
#include "protocol_call.h"


/* Configure Logging */
#define MODULE_NAME "callwind"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_NONE


static void _call_window_load(Window *window);
static void _call_window_unload(Window *window);
static void _call_window_draw(Layer *layer, GContext *ctx);
static void _call_window_animation_setup(Animation *animation);
static void _call_window_animation_update(Animation *animation,
                                  const AnimationProgress progress);
static void _call_window_animation_teardown(Animation *animation);
static bool _visible = false;
static uint8_t _current_command = 0;

void _call_window_animation_configure(Window *window, bool down);

void call_window_message_arrived(rebble_phone_message *call)
{
    _current_command = call->phone_message.command_id;
    appmanager_post_draw_message(1);
}

void call_window_overlay_display(OverlayWindow *overlay, Window *window)
{       
    notification_call *call = (notification_call*)window->context;
    _current_command = call->command_id;

    switch(call->command_id)
    {
        case PhoneMessage_IncomingCall:
            LOG_INFO("Incoming Call %s, %s", call->number, call->name);
            break;
        case PhoneMessage_MissedCall:
            LOG_INFO("Missed Call %s, %s", call->number, call->name);
            break;
        case PhoneMessage_CallStart:
            LOG_INFO("Call Started %s, %s", call->number, call->name);
            break;
        case PhoneMessage_CallEnd:
            LOG_INFO("Call End %s, %s", call->number, call->name);
            break;
        case PhoneMessage_Ring:
            LOG_INFO("Call Ring %s, %s", call->number, call->name);
            break;
        case PhoneMessage_PhoneStateResponse:
            LOG_INFO("Phone State");
            // payload is a pascal list of status packets. each list entry prefixed by length byte
            break;
        default:
            LOG_ERROR("Unknown Command %d", call->command_id);    
    }
    
    if (_visible)
    {
        /* if we are already visible, just request a redraw */
        window_dirty(true);
        return;
    }
    _visible = true;

    window->frame = call->frame;
    
    window_set_window_handlers(window, (WindowHandlers) {
        .load = _call_window_load,
        .unload = _call_window_unload,
    });
}

void call_window_overlay_destroy(OverlayWindow *overlay, Window *window)
{
    notification_call *call = (notification_call *)window->context;
    LOG_DEBUG("DESTROY %s", call->name);
    app_free(call->number);
//     app_free(call->name);
    app_free(window->context);
}

bool call_window_visible(void)
{
    return _visible;
}

static void _call_window_load(Window *window)
{
    notification_call *call = (notification_call *)window->context;
    
    _call_window_animation_configure(window, false);
        
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    layer_set_update_proc(layer, _call_window_draw);
    notification_load_click_config(window);
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _call_window_unload(Window *window)
{
    _visible = false;
}

static void _call_window_draw(Layer *layer, GContext *ctx)
{
    GRect wrect = layer->window->frame;
    //GRect wrect = GRect(0, -5, DISPLAY_COLS, 40);
    
    notification_call *call = (notification_call *)layer->window->context;

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, wrect, 0, GCornerNone);
    ctx->text_color = GColorWhite;
    GRect r = GRect(wrect.origin.x, wrect.origin.y + 20, wrect.size.w, wrect.size.h);
    
    switch(_current_command)
    {
        case PhoneMessage_IncomingCall:
            graphics_draw_text(ctx, "Incoming Call", fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
            r = GRect(wrect.origin.x, wrect.origin.y + 40, wrect.size.w, wrect.size.h);   
            graphics_draw_text(ctx, call->number, fonts_get_system_font(FONT_KEY_GOTHIC_18), r, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
            break;
        case PhoneMessage_CallStart:
            graphics_draw_text(ctx, "Talking", fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
            
            r = GRect(wrect.origin.x, wrect.origin.y + 40, wrect.size.w, wrect.size.h);
            graphics_draw_text(ctx, "00:00:12", fonts_get_system_font(FONT_KEY_GOTHIC_18), r, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
            break;
        case PhoneMessage_CallEnd:
            graphics_draw_text(ctx, "Call Ended", fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
            notification_reschedule_timer(layer_get_window(layer), 3000);
    }
    
    r = GRect(wrect.origin.x, wrect.origin.y + 20, wrect.size.w, wrect.size.h);
    graphics_draw_text(ctx, call->name, fonts_get_system_font(FONT_KEY_GOTHIC_18), r, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
}


void _call_window_animation_configure(Window *window, bool down)
{
    return;
    // Animate the window change
    Animation *animation = animation_create();
    animation_set_duration(animation, 1200);
    
    const AnimationImplementation implementation = {
        .setup = _call_window_animation_setup,
        .update = _call_window_animation_update,
        .teardown = _call_window_animation_teardown
    };
    animation_set_implementation(animation, &implementation);
    animation_set_context(animation, (void *)window);
    
    // Play the animation
    animation_schedule(animation);
}

static void _call_window_animation_setup(Animation *animation)
{
    SYS_LOG("call", APP_LOG_LEVEL_INFO, "Anim window ease in.");
}

static void _call_window_animation_update(Animation *animation,
                                  const AnimationProgress progress)
{
    Window *window = animation_get_context(animation); 
    assert(window);
    
    window->frame.origin.y = ANIM_LERP(DISPLAY_ROWS, DISPLAY_ROWS - 20, progress);
    
    window_dirty(true);
}


static void _call_window_animation_teardown(Animation *animation)
{
    SYS_LOG("call", APP_LOG_LEVEL_INFO, "Animation finished!");
    animation_destroy(animation);
}
