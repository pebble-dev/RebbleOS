/* watchdog.c
 * Watchdog timer care and feeding
 * RebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>, Joshua Wise <joshua@joshuawise.com>
 */

#include "platform.h" /* WATCHDOG_RESET_MS */
#include "FreeRTOS.h"
#include "task.h" /* xTaskCreate, vTaskDelay */

static StackType_t _watchdog_stack[configMINIMAL_STACK_SIZE];
static StaticTask_t _watchdog_task;
static void _threadmain_watchdog(void *pvParameters);

/* Early watchdog initialization.  Call as early as possible during boot --
 * starts the watchdog timer counting, and resets it once to allow for a
 * small period of time to get the system up and running.
 */
void rcore_watchdog_init_early() {
    hw_watchdog_init();
    hw_watchdog_reset();
}

/* Late watchdog initialization -- call once the RTOS is ready to safely
 * start allocating memory and creating tasks.
 */
void rcore_watchdog_init_late() {
    (void) xTaskCreateStatic(
        _threadmain_watchdog,             /* Function pointer */
        "rcore_watchdog",               /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 5UL,           /* Task priority */
        _watchdog_stack,                  /* Stack pointer */
        &_watchdog_task                   /* TCB memory */
        );
}

static void _threadmain_watchdog(void *pvParameters)
{
    while(1)
    {
        hw_watchdog_reset();
        vTaskDelay(WATCHDOG_RESET_MS / portTICK_RATE_MS);
    }
}
