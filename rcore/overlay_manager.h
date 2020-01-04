#pragma once
/**
 * @file overlay_manager.h
 * @author Barry Carter
 * @date 02 Feb 2018
 * @brief Overlay Windows management. You can create overlay windows over an app
 *
 * An \ref OverlayWindow is a special window that is drawn above a running app.
 * The memory and runtime of the \ref OverlayWindow are controlled away from the application
 * If you invoke one of these windows, expect little heap to play with.
 * 
 * You can create as many \ref OverlayWindow objects as you like, they will get stacked
 * like a normal window. In fact you can treat an \ref OverlayWindow as transparent
 * once you create one. Just deal with the underlying \ref Window object. You will know
 * with an assert if you do something that isn't allowed.
 * 
 * The \ref OverlayWindow has a child of a \ref Window object in it.
 * 
 * The window is isolated away from normal window functions, but please don't
 * try and push one of these windows into a normal apps running stack. It will break.
 * 
 * Window creation happens in a callback so we can delegate the work to our thread
 * and constrain memory and runtime of the window
 */
#include "rebbleos.h"

/* NOT USED YET */
typedef enum OverlayMode {
    /* Properly full screen, nothing underneath draws */
    OverlayModeFullScreenExclusive,
    OverlayModeOverlayContent,
    OverlayModeMoveContent,
} OverlayMode;

/**
 * @brief A special window that overlays all others
 */
typedef struct OverlayWindow {
    Window window;
    void *context;
    struct n_GContext *graphics_context;
    list_node node;
    list_head head;
} OverlayWindow;


/**
 * @brief Prototype for the callback on window creation
 * 
 * @param overlay An \ref OverlayWindow pointer with the newly created window
 */
typedef void (*OverlayCreateCallback)(OverlayWindow *overlay, Window *window);


/* Internal initialiser */
uint8_t overlay_window_init(void);

/* Internal. Check if any overlays or windows want a keypress */
bool overlay_window_accepts_keypress(void);
void overlay_window_post_button_message(ButtonMessage *message);


/**
 * @brief Creates a new managed \ref OverlayWindow.
 * 
 * The \ref OverlayWindow is provided in the callback \ref OverlayCreateCallback
 * @param creation_callback Is a provided function that will be called on the
 * creation of the window.
 */
void overlay_window_create(OverlayCreateCallback creation_callback);

/**
 * @brief Creates a new managed \ref OverlayWindow but also sets a custom context
 * By default context is set to the \ref OverlayWindow
 * 
 * The \ref OverlayWindow is provided in the callback \ref OverlayCreateCallback
 * @param creation_callback Is a provided function that will be called on the
 * creation of the window.
 */
void overlay_window_create_with_context(OverlayCreateCallback creation_callback, void *context);

/** 
 * @brief Directly draw an \ref OverlayWindow.
 * 
 * This will also cause a full redraw of all \ref Window objects
 * @param window_is_dirty When set the existing window we are overlaying 
 * is already dirty
 */
void overlay_window_draw(bool window_is_dirty);

/**
 * @brief Clean up an \ref OverlayWindow.
 * 
 * This will also cause a full redraw of all \ref Window objects
 * @param overlay_window Pointer to the \ref OverlayWindow you wish to destroy
 */
void overlay_window_destroy(OverlayWindow *overlay_window);

/**
 * @brief Get a count of all \ref OverlayWindow objects
 * @return uint8_t count of \ref OverlayWindow objects in existance
 */
uint8_t overlay_window_count(void);

/**
 * @brief Push a new \ref OverlayWindow to the top of the stack above all others
 * 
 * @param overlay_window Pointer to the \ref OverlayWindow to be pushed to the top
 * @param animated will enable any animated transistions on the window 
 */
void overlay_window_stack_push(OverlayWindow *overlay_window, bool animated);

/**
 * @brief Push a \ref OverlayWindow to the top of the stack. Push by \ref Window
 * 
 * @param window Pointer to the \ref Window to push to the top of the overlay
 * @param animated will enable any animated transistions on the window 
 */ 
void overlay_window_stack_push_window(Window *window, bool animated);

/**
 * @brief Pop a \ref OverlayWindow from the stack, returning the \ref Window removed
 * 
 * @param animated will enable any animated transistions on the window 
 */ 
Window *overlay_window_stack_pop_window(bool animated);

/**
 * @brief Return the top \ref Window in a stack of \ref OverlayWindow
 * 
 * @return \ref Window object of the found top window. NULL for no window.
 */
Window *overlay_window_stack_get_top_window(void);

/** 
 * @brief Given an \ref OverlayWindow, return the top \ref Window
 * 
 * @return \ref OverlayWindow that is very topmost
 */
OverlayWindow *overlay_stack_get_top_overlay_window(void);

/** 
 * @brief Given a \ref Window, check all \ref OverlayWindow objects for any match
 * 
 * @param window Pointer to the \ref Window to find the existance of
 * @return bool if window exists anywhere in an overlay
 */
bool overlay_window_stack_contains_window(Window *window);

/**
 * @brief Destroy an \ref OverlayWindow by reference of it's \ref Window
 * 
 * @param window Pointer to the \ref Window to destroy
 */
void overlay_window_destroy_window(Window *window);

/**
 * @brief Destroy an \ref OverlayWindow by reference of it's \ref OverlayWindow
 * 
 * @param animated If we should animate this overlay
 */
bool overlay_window_stack_remove(OverlayWindow *overlay_window, bool animated);

/**
 * @brief Get the next \ref Window available that has a \ref ClickConfigProvider
 * 
 * @return the pointer to the \ref Window with a click provider
 */ 
Window *overlay_window_get_next_window_with_click_config(void);




void overlay_await_draw_complete(void);


/**
 * @brief force a timer recalculation
 */
void overlay_timer_recalc(void);
