#pragma once
/**
 * @file overlay_manager_c.h
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
 * The \ref OverlayWindow has a list of \ref Window objects in it. You can push as many
 * \ref Window objects into an \ref OverlayWindow as you like. Memory permitting.
 * 
 * The window is isolated away from normal window functions, but please don't
 * try and push one of these windows into a normal apps running stack. It will break.
 * 
 * Window creation happens in a callback so we can delegate the work to our thread
 * and constrain memory and runtime of the window
 */
#include "rebbleos.h"
#include "overlay_manager.h"

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
    list_head window_list_head;
    list_node node;
} OverlayWindow;


/**
 * @brief Prototype for the callback on window creation
 * 
 * @param overlay An \ref OverlayWindow pointer with the newly created window
 */
typedef void (*OverlayCreateCallback)(OverlayWindow *overlay);

/* Internal initialiser */
void overlay_window_init(void);

/**
 * @brief Creates a new managed \ref OverlayWindow.
 * 
 * The \ref OverlayWindow is provided in the callback \ref OverlayCreateCallback
 * @param creation_callback Is a provided function that will be called on the
 * creation of the window.
 */
void overlay_window_create(OverlayCreateCallback creation_callback);

/** 
 * @brief Directly draw an \ref OverlayWindow.
 * 
 * This will also cause a full redraw of all \ref Window objects
 */
void overlay_window_draw(void);

/**
 * @brief Clean up an \ref OverlayWindow.
 * 
 * This will also cause a full redraw of all \ref Window objects
 * @param overlay_window Pointer to the \ref OverlayWindow you wish to destroy
 */
void overlay_window_destroy(OverlayWindow *overlay_window);

/**
 * @brief Get the \ref list_node linked list node's head
 * 
 * The head object contains a linked list of all \ref Window objects in this \ref OverlayWindow
 * @param window Pointer to the \ref Window you wish to find the head of
 */
list_head *overlay_window_thread_get_head(Window *window);

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
 * @brief Push a new \ref Window to the top of the \ref Window stack in an \ref OverlayWindow
 * 
 * @param overlay_window Pointer to the \ref OverlayWindow to push the \ref Window into
 * @param window Pointer to the \ref Window to push to the top of the overlay
 * @param animated will enable any animated transistions on the window 
 */ 
void overlay_window_stack_push_window(OverlayWindow *overlay_window, Window *window, bool animated);

/**
 * @brief Given an \ref OverlayWindow, return the top \ref Window
 * 
 * @param overlay_window Pointer to the \ref OverlayWindow to find the top window for
 * @return \ref Window object of the found top window. NULL for no window.
 */
Window *overlay_window_stack_get_top_window(OverlayWindow *overlay_window);

/** 
 * @brief Given an \ref OverlayWindow, return the top \ref Window
 * 
 * @return \ref OverlayWindow that is very topmost
 */
OverlayWindow *overlay_stack_get_top_overlay_window(void);
