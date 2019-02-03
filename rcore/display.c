/* display.c
 * routines for drawing to a display.
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

/* Notes
 * 
 * Calling display_draw starts the draw process.
 * display_draw => until hw is done => semaphore wait on isr
 * NOTES 
 *   The draw is run in the caller's thread context.
 *   This is a blocking process until a complete frame is drawn.
 *   This must be run in the scheduler, not before.
 *  
 */
 
#include "rebbleos.h"
#include "appmanager.h"

/* Semaphore to start drawing */
static SemaphoreHandle_t _display_start_sem;
static StaticSemaphore_t _display_start_sem_buf;

static void _display_start_frame(uint8_t offset_x, uint8_t offset_y);
static void _display_cmd(uint8_t cmd, char *data);

/* A mutex to use for locking buffers */
static StaticSemaphore_t _draw_mutex_buf;
static SemaphoreHandle_t _draw_mutex;

/*
 * Start the display driver and tasks
 */
uint8_t display_init(void)
{
    _display_start_sem = xSemaphoreCreateBinaryStatic(&_display_start_sem_buf);
    _draw_mutex        = xSemaphoreCreateMutexStatic(&_draw_mutex_buf);
    
    hw_display_init();
    os_module_init_complete(0);
    
    return INIT_RESP_ASYNC_WAIT;
}

/*
 * Called after the render of each row/col
 * We then set a semaphore for the display thread to wake on
 */
void display_done_isr(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* Notify the task that the transmission is complete. */
    xSemaphoreGiveFromISR(_display_start_sem, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*
 * Brutally and forcefully reset the display. This will
 * reset, but it will leave you dead in the water. Make sure to init.
 * or, instead, use init
 */
void display_reset(uint8_t enabled)
{
    hw_display_reset();
}

/*
 * Begin rendering a frame from the framebuffer into the display
 */
static void _display_start_frame(uint8_t xoffset, uint8_t yoffset)
{
    hw_display_start_frame(xoffset, yoffset);
}

/*
 * Get the pointer t the back buffer
 */
uint8_t *display_get_buffer(void)
{
    return hw_display_get_buffer();
}

/*
 * Queue a draw when available
 * This function starts the draw, and then sits and waits in 
 * a poll waiting for all frames to finish.
 * To be called from an rtos thread only
 */
void display_draw(void)
{
    uint8_t done = 0;
    _display_start_frame(0, 0);

    /* A frame is requested. Sit and await frame draw completion */
    while(!done)
    {
        /* block wait for the draw one a single row/col to finish
         * this is invoked via the ISR */
        xSemaphoreTake(_display_start_sem, portMAX_DELAY);
        done = hw_display_process_isr();
    }
}

inline bool display_buffer_lock_take(uint32_t timeout)
{
    return xSemaphoreTake(_draw_mutex, (TickType_t)timeout);
}

inline bool display_buffer_lock_give(void)
{
    return xSemaphoreGive(_draw_mutex);
}

bool display_is_buffer_locked(void)
{
    if(xSemaphoreTake(_draw_mutex, 0))
    {
        xSemaphoreGive(_draw_mutex);
        return false;
    }
    return true;
}
