/* window.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 *         Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "ngfxwrap.h"
#include "node_list.h"

node_t *_window_list_head;

/*
 * Create a new top level window and all of the contents therein
 */
Window *window_create(void)
{
    Window *window = calloc(1, sizeof(Window));
    
    if (window == NULL)
    {
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "No memory for Window");
        return NULL;
    }
    // and it's root layer
    GRect bounds = GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS);
    
    window->root_layer = layer_create(bounds);
    window->background_color = GColorWhite;

    return window;
}

/*
 * Set the pointers to the functions to call when the window is shown
 */
void window_set_window_handlers(Window *window, WindowHandlers handlers)
{
    if (window == NULL)
        return;
    
    window->window_handlers = handlers;
}

static void push_animation_setup(Animation *animation) {
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "Animation started!");
    
    /*Window *top = top_window->window;
    Layer *previous = top_window->previous->window->root_layer;
    layer_add_child(top->root_layer, previous);*/
}

static void push_animation_update(Animation *animation,
                                  const AnimationProgress progress) {
    // Animate some completion variable
    int s_animation_percent = ((int)progress * 100) / ANIMATION_NORMALIZED_MAX;
    
//     Window *wind = (Window*)(_window_list_head->data);
//     Layer *previous = top_window->previous->window->root_layer;
    /*
    previous->bounds = GRect(0 - ((DISPLAY_COLS / 100) * s_animation_percent), previous->bounds.origin.y, previous->bounds.size.w, previous->bounds.size.h);
    top->root_layer->bounds = GRect(DISPLAY_COLS - ((DISPLAY_COLS / 100) * s_animation_percent), top->root_layer->bounds.origin.y, top->root_layer->bounds.size.w, top->root_layer->bounds.size.h);*/
    
    window_dirty(true);
}

static void push_animation_teardown(Animation *animation) {
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "Animation finished!");
//     Layer *previous = top_window->previous->window->root_layer;
//     layer_remove_from_parent(previous);
    //layer_add_child(previous, prev)
}

/*
 * Push a window onto the main window window_stack_push
 */
void window_stack_push(Window *window, bool animated)
{
    node_add(&_window_list_head, window);
    
    if (animated)
    {
        // Animate the window change
        Animation *animation = animation_create();
        animation_set_duration(animation, 1000);
        
        const AnimationImplementation implementation = {
            .setup = push_animation_setup,
            .update = push_animation_update,
            .teardown = push_animation_teardown
        };
        animation_set_implementation(animation, &implementation);
        
        // Play the animation
        //animation_schedule(animation);
    }
    window_configure();
    window_dirty(true);
}

/*
 * Remove the top_window from the list
 */
Window * window_stack_pop(bool animated)
{
    Window *wind = window_stack_get_top_window();
    window_stack_remove(wind, animated);
    
    return wind;
}

/*
 * Remove a window from the list
 */
bool window_stack_remove(Window *window, bool animated)
{
    node_remove(&_window_list_head, window);

    return true;
}

/*
 * Get the topmost window
 */
Window * window_stack_get_top_window(void)
{
    node_t *cur = _window_list_head;
    
    if (_window_list_head == NULL)
        return NULL;    
       
    /* fast forward to the node we want to find */
    while(cur != NULL && cur->next != NULL)
        cur = cur->next;
    return (Window *)cur->data;
}

uint16_t window_count(void)
{
    uint16_t count = 0;
    node_t *cur = _window_list_head;
    
    if (_window_list_head == NULL)
        return 0;
    
    /* fast forward to the node we want to find */
    while(cur != NULL && cur->next != NULL)
    {
        cur = cur->next;
        count++;
    }
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "COUNT %d", count);
    
    return count + 1;
}

/*
 * Check if a window exists in the stack
 */
bool window_stack_contains_window(Window *window)
{
    if (node_find(&_window_list_head, window) != NULL)
        return true;
    
    return false;
}

/*
 * Kill a window. Clean up references and memory
 */
void window_destroy(Window *window)
{
    // free all of the layers
    layer_destroy(window->root_layer);
    // and now the window
    app_free(window);
}

/*
 * Get the top level window's layer
 */
Layer *window_get_root_layer(Window *window)
{
    if (window == NULL)
        return NULL;
    
    return window->root_layer;
}


/*
 * Invalidate the window so it is scheduled for a redraw
 */
void window_dirty(bool is_dirty)
{
    Window *wind = window_stack_get_top_window();
    
    if (wind == NULL)
        return;
    
    if (wind->is_render_scheduled != is_dirty)
    {
        wind->is_render_scheduled = is_dirty;
        
        if (is_dirty)
            appmanager_post_draw_message();
    }
}

void window_draw()
{
    Window *wind = window_stack_get_top_window();
    
    if (wind == NULL)
        return;
    if (wind && wind->is_render_scheduled)
    {
        GContext *context = rwatch_neographics_get_global_context();
        GRect frame = layer_get_frame(wind->root_layer);
        context->offset = frame;
        context->fill_color = wind->background_color;
        graphics_fill_rect_app(context, GRect(0, 0, frame.size.w, frame.size.h), 0, GCornerNone);
        
        walk_layers(wind->root_layer, context);
        
        rbl_draw();
        wind->is_render_scheduled = false;
    }
}

/*
 * Window click config provider registration implementation.
 * For the most part, these will just defer to the button recogniser
 */

void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider)
{
    window->click_config_provider = click_config_provider;
}

void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context)
{
    window->click_config_provider = click_config_provider;
    window->click_config_context = context;
}

ClickConfigProvider window_get_click_config_provider(const Window *window)
{
    return window->click_config_provider;
}

void *window_get_click_config_context(Window *window)
{
    return window->click_config_context;
}

void window_set_background_color(Window *window, GColor background_color)
{
    window->background_color = background_color;
}

bool window_is_loaded(Window *window)
{
    return window->is_loaded;
}

void window_set_user_data(Window *window, void *data)
{
    window->user_data = data;
}

void * window_get_user_data(const Window *window)
{
    return window->user_data;
}

void window_single_click_subscribe(ButtonId button_id, ClickHandler handler)
{
    button_single_click_subscribe(button_id, handler);
}

void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler)
{
    button_single_repeating_click_subscribe(button_id, repeat_interval_ms, handler);
}

void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler)
{
    button_multi_click_subscribe(button_id, min_clicks, max_clicks, timeout, last_click_only, handler);
}

void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler)
{
    button_long_click_subscribe(button_id, delay_ms, down_handler, up_handler);
}

void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context)
{
    button_raw_click_subscribe(button_id, down_handler, up_handler, context);
}

void window_set_click_context(ButtonId button_id, void *context)
{
    button_set_click_context(button_id, context);
}


/*
 * Deal with window level callbacks when a window is created.
 * These will call through to the pointers to the functions in
 * the user supplied window
 */
void window_configure(void)
{
    // we assume they are configured now
    rbl_window_load_proc();
    rbl_window_load_click_config();
}

void rbl_window_load_proc(void)
{
    Window *wind = window_stack_get_top_window();
    
    if (_window_list_head && wind->window_handlers.load)
        wind->window_handlers.load(wind);
}

void rbl_window_load_click_config(void)
{
    Window *wind = window_stack_get_top_window();
    
    if (wind->click_config_provider) {
        void* context = wind->click_config_context ? wind->click_config_context : wind;
        wind->click_config_provider(context);
    }
}

