/* overlay_manager.c
 * Routines for managing the overlay window and it's content
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "overlay_manager.h"

/* A message to talk to the overlay thread */
typedef struct OverlayMessage {
    uint8_t command;
    void *data;
} OverlayMessage;

#define OVERLAY_CREATE    0
#define OVERLAY_DRAW      1
#define OVERLAY_DESTROY   2

static xQueueHandle _overlay_queue;
static void _overlay_thread(void *pvParameters);
static list_head _overlay_window_list_head = LIST_HEAD(_overlay_window_list_head);
static void _overlay_window_draw(void);

void overlay_window_init(void)
{
    _overlay_queue = xQueueCreate(1, sizeof(struct OverlayMessage));
    
    app_running_thread *thread = appmanager_get_thread(AppThreadOverlay);
    thread->status = AppThreadLoaded;
    thread->thread_entry = &_overlay_thread;
    /* start the thread 
     * We are only using the thread launcher in appmanager for this
     * not the full supervisory process. It's lightweight
     */
    appmanager_execute_app(thread, 0);    
}

/*
 * Create a new top level window and all of the contents therein
 */
void overlay_window_create(OverlayCreateCallback creation_callback)
{
    OverlayMessage om = (OverlayMessage) {
        .command = OVERLAY_CREATE,
        .data = (void *)creation_callback
    };
    xQueueSendToBack(_overlay_queue, &om, 0);
}

void overlay_window_draw(void)
{
    OverlayMessage om = (OverlayMessage) {
        .command = OVERLAY_DRAW,
    };
    xQueueSendToBack(_overlay_queue, &om, 0);
}


void overlay_window_destroy(OverlayWindow *overlay_window)
{
        OverlayMessage om = (OverlayMessage) {
        .command = OVERLAY_DESTROY,
        .data = (void *)overlay_window
    };
    xQueueSendToBack(_overlay_queue, &om, 0);
}

list_head *overlay_window_thread_get_head(Window *window)
{
    OverlayWindow *ow;
    Window *w;
    list_foreach(ow, &_overlay_window_list_head, OverlayWindow, node)
    {
        list_foreach(w, &ow->window_list_head, Window, node)
        {
            if (w == window)
            {
                return &ow->window_list_head;
            }
        }
    }
    
    return NULL;
}

Window *overlay_window_stack_get_top_window(OverlayWindow *overlay_window)
{
    list_head *lh = &overlay_window->window_list_head;
    return list_elem(list_get_head(lh), Window, node);
}

OverlayWindow *overlay_stack_get_top_overlay_window(void)
{
    return list_elem(list_get_head(&_overlay_window_list_head), OverlayWindow, node);
}

list_head *overlay_window_get_list_head(void)
{
    return &_overlay_window_list_head;
}

uint8_t overlay_window_count(void)
{
    uint16_t count = 0;
    
    OverlayWindow *w;
    list_foreach(w, &_overlay_window_list_head, OverlayWindow, node)
    {
        count++;
    }
    
    return count;
}

void overlay_window_stack_push(OverlayWindow *overlay_window, bool animated)
{
    list_init_node(&overlay_window->node);
    list_insert_head(&_overlay_window_list_head, &overlay_window->node);
    
    Window* window = overlay_window_stack_get_top_window(overlay_window);
    assert(window);
    window->is_render_scheduled = true;
    window_dirty(true);
}

void overlay_window_stack_push_window(OverlayWindow *overlay_window, Window *window, bool animated)
{
    list_init_node(&window->node);
    list_insert_head(&overlay_window->window_list_head, &window->node);
    window_stack_push_configure(window, animated);
}

static void _overlay_window_create(OverlayCreateCallback create_callback)
{
    OverlayWindow *overlay_window = app_calloc(1, sizeof(OverlayWindow));
    assert(overlay_window && "No memory for Overlay window");
    list_init_head(&overlay_window->window_list_head);
    
    /* invoke creation callback so it can be drawn in the right heap */
    ((OverlayCreateCallback)create_callback)(overlay_window);
}

static void _overlay_window_destroy(OverlayWindow *overlay_window)
{                   
    Window *w;
    list_node *l = list_get_head(&overlay_window->window_list_head);
    while(l)
    {
        w = list_elem(l, Window, node);
        list_remove(&overlay_window->window_list_head, &w->node);
        window_destroy(w);
        l = list_get_head(&overlay_window->window_list_head);
        if (l->prev == l->next)
        {
            SYS_LOG("overlay", APP_LOG_LEVEL_ERROR, "Deleted all overlay windows");
            break;
        }
    }
    
    list_remove(&_overlay_window_list_head, &overlay_window->node);
    app_free(overlay_window);
    
    Window *top_window;
    if (overlay_window_count() == 0)
    {
        /* we are out of overlay windows, restore click 
         * first get the top normal window. Grab it's click context
         * then restore it. Then configure it. */
        top_window = window_stack_get_top_window();
    }
    else
    {
        /* We still have overlay windows, so we set the click 
         * config handler to the topmost OverlayWindow.
         * Then get that ow's top Window. */
        OverlayWindow *ow = overlay_stack_get_top_overlay_window();
        top_window = overlay_window_stack_get_top_window(ow);
    }
    if (top_window)
        window_load_click_config(top_window);
}

static void _overlay_window_draw(void)
{
    if (appmanager_get_thread_type() != AppThreadOverlay)
    {
        SYS_LOG("window", APP_LOG_LEVEL_ERROR, "Someone not overlay thread is trying to draw. Tsk.");
        return;
    }
    
    overlay_window_count();
    OverlayWindow *ow;
    list_foreach(ow, &_overlay_window_list_head, OverlayWindow, node)
    {
        Window *window = overlay_window_stack_get_top_window(ow);
        assert(window);
        
        rbl_window_draw(window);
        window->is_render_scheduled = false;
    }
    
    rbl_draw();        
}

static void _overlay_thread(void *pvParameters)
{
    OverlayMessage data;
    
    SYS_LOG("overlay", APP_LOG_LEVEL_INFO, "Starting overlay thread...");
    
    while(1)
    {
        if (xQueueReceive(_overlay_queue, &data, portMAX_DELAY))
        {
            switch(data.command)
            {
                case OVERLAY_CREATE:
                    assert(data.data && "You MUST provide a callback");
                    _overlay_window_create((OverlayCreateCallback)data.data);                    
                    break;
                case OVERLAY_DRAW:
                    _overlay_window_draw();
                    break;
                case OVERLAY_DESTROY:
                    assert(data.data && "You MUST provide a valid overlay window");
                    OverlayWindow *ow = (OverlayWindow *)data.data;
                    _overlay_window_destroy(ow);
                    break;
                default:
                    assert(!"I don't know this command!");
            }
        }
    }
}
