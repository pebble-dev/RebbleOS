#include "rebbleos.h"
#include "protocol_notification.h"
#include "notification_manager.h"
#include "platform_res.h"

static void _minimsg_window_load(Window *window);
static void _minimsg_window_unload(Window *window);
static void _draw_mini_message(Layer *layer, GContext *ctx);
static void _minimsg_animation_setup(Animation *animation);
static void _minimsg_animation_update(Animation *animation,
                                  const AnimationProgress progress);
static void _minimsg_animation_teardown(Animation *animation);
void _minimsg_animation_configure(bool down);

void mini_message_overlay_display(OverlayWindow *overlay, Window *window)
{
    notification_mini_msg *message = (notification_mini_msg*)window->context;

    window->frame = message->frame;
    window->background_color = GColorWhite;
    
    window_set_window_handlers(window, (WindowHandlers) {
        .load = _minimsg_window_load,
        .unload = _minimsg_window_unload,
    });
}

void mini_message_overlay_destroy(OverlayWindow *overlay, Window *window)
{      

}

static void _minimsg_window_load(Window *window)
{
    notification_mini_msg *message = (notification_mini_msg*)window->context;
    
    _minimsg_animation_configure(false);
        
    Layer *layer = window_get_root_layer(window);    
    GRect bounds = layer_get_unobstructed_bounds(layer);

    layer_set_update_proc(layer, _draw_mini_message);
   
    layer_mark_dirty(layer);
    window_dirty(true);
}

static void _minimsg_window_unload(Window *window)
{
    notification_mini_msg *nm = (notification_mini_msg *)window->context;
    noty_free(window->context);
}

static void _draw_mini_message(Layer *layer, GContext *ctx)
{
    GRect wrect = GRect(0, -5, DISPLAY_COLS, 20);
    
    notification_mini_msg *message = (notification_mini_msg*)layer->window->context;

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, wrect, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_draw_line(ctx, GPoint(0, -5), GPoint(DISPLAY_COLS, -5));
    
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_rect(ctx, GRect(0, -4, DISPLAY_COLS, 3), 0, GCornerNone);
    
//     graphics_context_set_fill_color(ctx, GColorRed);
//     graphics_fill_rect(ctx, GRect(50, -8, DISPLAY_COLS - 100, 6), 0, GCornerNone);
           
    ctx->text_color = GColorBlack;
    graphics_draw_text(ctx, message->message , fonts_get_system_font(FONT_KEY_GOTHIC_18), wrect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
}


void _minimsg_animation_configure(bool down)
{
    // Animate the window change
    Animation *animation = animation_create();
    animation_set_duration(animation, 1200);
    
    const AnimationImplementation implementation = {
        .setup = _minimsg_animation_setup,
        .update = _minimsg_animation_update,
        .teardown = _minimsg_animation_teardown
    };
    animation_set_implementation(animation, &implementation);
    
    // Play the animation
    animation_schedule(animation);
}

static void _minimsg_animation_setup(Animation *animation)
{
    SYS_LOG("minimsg", APP_LOG_LEVEL_INFO, "Anim window ease in.");
}

static void _minimsg_animation_update(Animation *animation,
                                  const AnimationProgress progress)
{
    Window *window = overlay_window_stack_get_top_window(); 
    
    window->frame.origin.y = ANIM_LERP(DISPLAY_ROWS, DISPLAY_ROWS - 20, progress);
    
    window_dirty(true);
}


static void _minimsg_animation_teardown(Animation *animation)
{
    SYS_LOG("minimsg", APP_LOG_LEVEL_INFO, "Animation finished!");
    animation_destroy(animation);
}
