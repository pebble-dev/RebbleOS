/* display.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
 
#include "rebbleos.h"

static TaskHandle_t _display_task;
static xQueueHandle _display_queue;
static SemaphoreHandle_t _display_mutex;
static StaticSemaphore_t _display_mutex_buf;

static void _display_thread(void *pvParameters);
static void _display_start_frame(uint8_t offset_x, uint8_t offset_y);
static void _display_cmd(uint8_t cmd, char *data);

/*
 * Start the display driver and tasks. Show splash
 */
void display_init(void)
{   
    hw_display_init();
      
    // set up the RTOS tasks
    xTaskCreate(_display_thread, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &_display_task);
    
    _display_queue = xQueueCreate(2, sizeof(uint8_t));
    _display_mutex = xSemaphoreCreateMutexStatic(&_display_mutex_buf);
    
    _display_cmd(DISPLAY_CMD_DRAW, NULL);
    
    KERN_LOG("Display", APP_LOG_LEVEL_INFO, "Display Tasks Created");
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

    // Notify the task that the transmission is complete.
    vTaskNotifyGiveFromISR(_display_task, &xHigherPriorityTaskWoken);

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
    xSemaphoreTake(_display_mutex, portMAX_DELAY);
    
    hw_display_start_frame(xoffset, yoffset);
    
    // block wait for the draw to finish
    // this is invoked via the ISR
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    // unlock the mutex
    xSemaphoreGive(_display_mutex);
}

/*
 * Get the pointer t the back buffer
 */
uint8_t *display_get_buffer(void)
{
    return hw_display_get_buffer();
}

/*
 * Request a command from the display driver. 
 * Such as DISPLAY_CMD_DRAW
 */
static void _display_cmd(uint8_t cmd, char *data)
{
    xQueueSendToBack(_display_queue, &cmd, 0);
}

/*
 * Queue a draw when available
 */
void display_draw(void)
{
    _display_cmd(DISPLAY_CMD_DRAW, 0);
}

/*
 * Main task processing for the display. Manages locking
 * state machine control and command management
 */
static void _display_thread(void *pvParameters)
{
    uint8_t data;
    const TickType_t max_block_time = pdMS_TO_TICKS(1000);

    // XXX Assume once screen is up, we are up.
    rebbleos_set_system_status(SYSTEM_STATUS_STARTED);
    
    while(1)
    {
        // commands to be executed are send to this queue and processed
        // one at a time
        if (xQueueReceive(_display_queue, &data, max_block_time))
        {
            switch(data)
            {
                // in the case of draw, we are going to leave locking to
                // the outer laters. If someone calls an overlapping draw into here
                // it's just going to fail
                case DISPLAY_CMD_DRAW:
                    // all we are responsible for is starting a frame draw
                    _display_start_frame(0, 0);
                    break;
                case DISPLAY_CMD_DONE:
                    break;
            }
        }
        else
        {
            // nothing emerged from the buffer
        }        
    }
}
