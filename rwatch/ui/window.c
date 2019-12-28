/* routines for Managing the Window object
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 *         Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "ngfxwrap.h"
#include "node_list.h"
#include "property_animation.h"
#include "animation.h"
#include "overlay_manager.h"
#include "notification_manager.h"

static list_head _window_list_head = LIST_HEAD(_window_list_head);

static void _window_load_proc(Window *window);

static bool _anim_direction_left = true;
static void _animation_util_push_fb(GRect rect, int16_t distance);
static void _animation_setup(bool direction_left);
static void _push_animation_update(Animation *animation,
                                  const AnimationProgress progress);


/*
 * Create a new top level window and all of the contents therein
 */
Window *window_create(void)
{
    Window *window = app_calloc(1, sizeof(Window));

    if (window == NULL)
    {
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "No memory for Window");
        return NULL;
    }

    window_ctor(window);
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "ctor 0x%x", window);
    return window;
}

void window_ctor(Window *window)
{
    GRect bounds = GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS);
    GRect frame = GRect(0, 0, DISPLAY_COLS, DISPLAY_ROWS);
    window->frame = frame;
    window->root_layer = layer_create(bounds);
    window->root_layer->window = window;
    window->background_color = GColorWhite;
    window->load_state = WindowLoadStateUnloaded;
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "CTOR");
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

static void _push_animation_setup(Animation *animation) {
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "Anim window ease in.");
}

static void _push_animation_update(Animation *animation,
                                  const AnimationProgress progress)
{
    Window *window = window_stack_get_top_window(); 
    int existx = window->frame.origin.x; 
    int newx, delta; 
     
    if (*((bool*)animation->context) == true) 
    { 
        newx = ANIM_LERP(DISPLAY_COLS, 0, progress) - 1; 
        delta = (existx - newx);
        if (delta && existx > 0)
        {
            _animation_util_push_fb(GRect(delta, 0, DISPLAY_COLS - delta, DISPLAY_ROWS), -delta);
        }
    } 
    else 
    {
        newx = ANIM_LERP(-DISPLAY_COLS, 0, progress);
        delta = newx - existx;
        if (delta && existx < 0)
            _animation_util_push_fb(GRect(DISPLAY_COLS + existx + 1, 0, DISPLAY_COLS - delta + 1, DISPLAY_ROWS), delta);
    } 

    window->frame.origin.x = newx; 
    window_dirty(true); 
}

static void _push_animation_teardown(Animation *animation) {
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "Animation finished!");
    animation_destroy(animation);
}


/* 
 * Get the head node for a window list
 */
list_head *window_thread_get_head(Window *window)
{
    int ttype = appmanager_get_thread_type();
    if (ttype == AppThreadMainApp)
        return &_window_list_head;
    else if (ttype == AppThreadOverlay)
        return overlay_window_get_list_head();
    
    assert(!"I don't know how to deal with a window in this thread!");
    return NULL;
}


/*
 * Push a window onto the main window window_stack_push
 */
void window_stack_push(Window *window, bool animated)
{
    if(appmanager_is_thread_overlay())
    {
        overlay_window_stack_push_window(window, animated);
        return;
    }

    list_init_node(&window->node);
    /* It is only valid to window push into a window */
    list_insert_head(&_window_list_head, &window->node);
    window_stack_push_configure(window, animated);
}

void window_stack_push_configure(Window *window, bool animated)
{
    if (animated)
    {
        Window *window = window_stack_get_top_window();
        window->frame.origin.x = DISPLAY_COLS;
        App *app = appmanager_get_current_app();
        /* A quicky hack to determine direction of scroll
         * If we are an app => face, then we go left
         * Face to app => right
         */
        if (app->type == APP_TYPE_FACE)
            _animation_setup(true);
        else 
            _animation_setup(false);
    }

    window_configure(window);
    window_dirty(true);
}

static void _animation_setup(bool direction_left)
{
    // Animate the window change
    Animation *animation = animation_create();
    animation_set_duration(animation, 1200);
    
    const AnimationImplementation implementation = {
        .setup = _push_animation_setup,
        .update = _push_animation_update,
        .teardown = _push_animation_teardown
    };
    animation_set_implementation(animation, &implementation);
 
    _anim_direction_left = direction_left;
    animation->context = (void*)&_anim_direction_left;
    
    // Play the animation
    animation_schedule(animation);
}

/*
 * Remove the top_window from the list
 */
Window * window_stack_pop(const bool animated)
{
    assert(appmanager_get_thread_type() != AppThreadOverlay 
            && "Please use overlay_window_stack_pop");
    
    Window *wind = window_stack_get_top_window();
    window_stack_remove(wind, animated);

    return wind;
}

void window_stack_pop_all(const bool animated)
{
    assert(appmanager_get_thread_type() != AppThreadOverlay 
            && "Please use overlay_window_stack_pop_all");

    Window *wind = window_stack_get_top_window();

    while(wind)
    {
        window_stack_remove(wind, animated);
        wind = window_stack_get_top_window();
    }
}

/*
 * Remove a window from the list
 */
bool window_stack_remove(Window *window, bool animated)
{
    if (overlay_window_stack_contains_window(window))
        return overlay_window_stack_remove(container_of(window, OverlayWindow, window), animated);
    
    list_head *lh = window_thread_get_head(window);
    Window* top_window = list_elem(list_get_head(lh), Window, node);
    list_remove(lh, &window->node);

    if (top_window == window) {
        top_window = list_elem(list_get_head(lh), Window, node);
        if (top_window) {
            window_configure(top_window);
            window_dirty(true);
        }
    }

    return true;
}

/*
 * Get the topmost window
 */
Window *window_stack_get_top_window(void)
{
    return list_elem(list_get_head(&_window_list_head), Window, node);
}

uint16_t window_count(void)
{
    uint16_t count = 0;
    Window *w;
    
    if (appmanager_is_thread_overlay())
        return overlay_window_count();
        
    if (list_get_head(&_window_list_head) == NULL)
        return 0;
    
    list_foreach(w, &_window_list_head, Window, node)
    {
        count++;
    }
    
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "COUNT %d", count);
    
    return count;
}

/*
 * Check if a window exists in the stack
 */
bool window_stack_contains_window(Window *window)
{
    if (overlay_window_stack_contains_window(window))
        return true;
            
    list_head *lh = window_thread_get_head(window);
    Window *w;
    
    list_foreach(w, lh, Window, node)
    {
        if (window == w)
            return true;
    }
    
    return false;
}

/*
 * Kill a window. Clean up references and memory
 */
void window_destroy(Window *window)
{
    uint8_t _count;
    
    if (overlay_window_stack_contains_window(window))
    {
        overlay_window_destroy_window(window);
        return;
    }
    
    list_head *lh = window_thread_get_head(window);
    
    if (window->load_state != WindowLoadStateLoaded &&
        window->load_state != WindowLoadStateUnloaded
    )
    {
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "Window is either loading or unloading!!");
        return;
    }
    
    /* Check the node isn't already detached */
    if (!(window->node.next == NULL && window->node.prev == NULL))
        list_remove(lh, &window->node);

    /* Unload the window */
    _window_unload_proc(window);
    window_dtor(window);
    /* and now the window */
    app_free(window);
    
    _count = window_count();  
    
    if (_count == 0)
    {
        SYS_LOG("window", APP_LOG_LEVEL_INFO, "No more windows!");
        return;
    }
    
    /* Clean up the clink handler and remap back to our current window */
    window_load_click_config(window_stack_get_top_window());
    window_dirty(true);
}

void window_dtor(Window* window)
{
    // free all of the layers
    layer_destroy(window->root_layer);
    app_free(window->root_layer);
    window->root_layer = NULL;
    SYS_LOG("window", APP_LOG_LEVEL_INFO, "DTOR");
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
    
    if (!wind)
        return;

    wind->is_render_scheduled = is_dirty;
}

/* 
 * Draw a window.
 */
void rbl_window_draw(Window *window)
{
    assert(window && "Invalid window to draw");

    GContext *context = rwatch_neographics_get_global_context();
    GRect frame = layer_get_frame(window->root_layer);
    GRect windowframe = window->frame; 
    frame.origin.y += windowframe.origin.y; 
    frame.origin.x += windowframe.origin.x; 
    /* Apply window offset too */
    context->offset = frame;
    context->fill_color = window->background_color;
    graphics_fill_rect(context, GRect(0, 0, frame.size.w, frame.size.h), 0, GCornerNone);
    layer_draw(window->root_layer, context);
}

/*
 * Draw the window, which in general means painting the background
 * and then walking all layers and drawing them
 * 
 * This will draw and lock all further calls behind a mutex.
 * Once the app layers are drawn, a request is sent to the overlay
 * thread to draw itself.
 * After the request is sent, we sleep the thread awaiting wakeup from the 
 * overlay draw completion.
 * Overlay will call to paint out the framebuffer.
 * Display drawing is async, so if a display draw is already in progress, we 
 * also wait for that to complete.
 */
bool window_draw(void)
{
    if (appmanager_is_thread_overlay())
    {
        return false;
    }
    else if (appmanager_get_thread_type() != AppThreadMainApp)
    {
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "XXX Not app thread! I don't trust you to allocate memory correctly.");
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "XXX Please find the correct mechanism! (did you mean overlay_x?).");
        return false;
    }

    Window *wind = window_stack_get_top_window();

    rbl_window_draw(wind);
    wind->is_render_scheduled = false;
    
    return true;
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
    return window->load_state == WindowLoadStateLoaded;
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
void window_configure(Window *window)
{
    // we assume they are configured now
    _window_load_proc(window);
    if (window)
        window_load_click_config(window);
}

/* 
 * Call the window's _load handler and flag as loaded
 */
void _window_load_proc(Window *window)
{
    if (window->load_state == WindowLoadStateLoaded ||
        window->load_state == WindowLoadStateLoading
    )
        return;
    
    /* we should flag as loaded even if they don't have a load handler */ 
    window->load_state = WindowLoadStateLoading;
         
    if (window->window_handlers.load)
        window->window_handlers.load(window);
    
    window->load_state = WindowLoadStateLoaded;
}

/* 
 * Call the window's _unload handler and flag as unloaded
 */
void _window_unload_proc(Window *window) 
{
    if (window->load_state != WindowLoadStateLoaded)
        return;
    
    window->load_state = WindowLoadStateUnloading;
    
    if (window->window_handlers.unload)
        window->window_handlers.unload(window);
    
    /* we should flag as unloaded even if they don't have a load handler */
    window->load_state = WindowLoadStateUnloaded;
    return;
}

void window_load_click_config(Window *window)
{
    /* A window is being configured. If it is a normal window and we are
     * in an overlay thread, ignore */
    /* Commented out for now to give the overlays and notifications
     * opportunity to override clicks instead of exclusively owning them
    if (appmanager_is_thread_app() && overlay_window_accepts_keypress())
    {
        return;
    }*/
   
    if (window->click_config_provider) {
        void* context = window->click_config_context ? window->click_config_context : window;
        for (int i = 0; i < NUM_BUTTONS; i++) {
            window_single_click_subscribe(i, NULL);
            window_long_click_subscribe(i, 0, NULL, NULL);
            window_multi_click_subscribe(i, 0, 0, 0, false, NULL);
            window_raw_click_subscribe(i, NULL, NULL, context);
        }
        window->click_config_provider(context);
    } 
}


/* This prob shouldn't be in here, but I feel it doesn't live in 
   animation either */

/* 
 * Grab the screenbuffer and push it off the screen left or right by n
 * pixels.
 */
static void _animation_util_push_fb(GRect rect, int16_t distance)
{
#ifdef PBL_BW
#  warning XXX: PBL_BW no push_fb support
    return;
#else
    /* To revisit at some later time.
     * The issue I can't find time to fix is that pushfb happens
     * while we are painting */
    return;
    
    uint8_t *fb = display_get_buffer();
    if (!display_buffer_lock_take(0))
        return;
    
    if (rect.origin.x < 0) rect.origin.x = 0;
    if (rect.origin.x > DISPLAY_COLS) rect.origin.x = DISPLAY_COLS;
    if (rect.origin.y < 0) rect.origin.y = 0;
    if (rect.origin.y > DISPLAY_ROWS) rect.origin.y = DISPLAY_ROWS;
    
    uint8_t *origin = &fb[(rect.origin.y * DISPLAY_COLS) + rect.origin.x];
    uint8_t *p;
    int8_t incr = distance > 0 ? -1 : 1;

    for (int16_t y = 0; y < rect.size.h; y++)
    {
        uint16_t x = distance > 0 ? rect.size.w - 1 : 0;
       
        p = origin + (y * DISPLAY_COLS) + x;
        
        for (;;)
        {      
            *(p + distance) = *p;
            
            x += incr;
            p += incr;
            
            if (distance < 0 && x >= rect.size.w)
                break;
            else if (distance > 0 && x <= 0)
                break;
            
            if ((p + distance > origin + (DISPLAY_COLS * rect.size.h) + rect.size.w) ||
                p > origin + DISPLAY_ROWS * DISPLAY_COLS)
                continue;
        }
        
    }
    display_buffer_lock_give();
#endif
}

bool window_get_fullscreen(Window *window)
{
    return true;
}

void window_set_fullscreen(Window *window)
{
    return;
}
