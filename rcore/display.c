/* display.c
 * routines for drawing to a display
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
 
#include "rebbleos.h"
#include "appmanager.h"

/* Semaphore to start drawing */
static SemaphoreHandle_t _display_start_sem;
static StaticSemaphore_t _display_start_sem_buf;

static void _display_thread(void *pvParameters);
static void _display_start_frame(uint8_t offset_x, uint8_t offset_y);
static void _display_cmd(uint8_t cmd, char *data);

/* A mutex to use for locking buffers */
static StaticSemaphore_t _draw_mutex_buf;
static SemaphoreHandle_t _draw_mutex;

/*
 * Start the display driver and tasks. Show splash
 */
uint8_t display_init(void)
{
    _display_start_sem = xSemaphoreCreateBinaryStatic(&_display_start_sem_buf);
    _draw_mutex    = xSemaphoreCreateMutexStatic(&_draw_mutex_buf);
    
    hw_display_init();
    os_module_init_complete(0);
    
    return INIT_RESP_ASYNC_WAIT;
}

/*
 * When the display driver has responded to something
 * 1) frame frame accepted
 * 2) frame completed
 * We get notified here. We can now let the prcessing complete or continue
 */
void display_done_ISR(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    xSemaphoreGiveFromISR(_display_start_sem, &xHigherPriorityTaskWoken);
    
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
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
 */
void display_draw(void)
{
    _display_start_frame(0, 0);

    /* block wait for the draw to finish
     * this is invoked via the ISR */
    xSemaphoreTake(_display_start_sem, portMAX_DELAY);
}

inline bool display_buffer_lock_take(uint32_t timeout)
{
    return xSemaphoreTake(_draw_mutex, (TickType_t)timeout);
}

inline bool display_buffer_lock_give(void)
{
    return xSemaphoreGive(_draw_mutex);
}
