/* main.c
 * Main entry point
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "watchdog.h"
#include "ambient.h"

int main(void)
{
    SystemInit();

    hardware_init();

    rebbleos_init();
    
    KERN_LOG("main", APP_LOG_LEVEL_INFO, "RebbleOS %s", VERSION);
    
    vTaskStartScheduler();  // should never return
    
    panic("vTaskStartScheduler returned?");
}


/*
 * Initialise the whole Rebble platform
 */
void hardware_init(void)
{
    platform_init();
    debug_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Debug Init");
    rcore_watchdog_init_early();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Watchdog Init");
    power_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Power Init");
    flash_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Flash Init");
    vibrate_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Vibro Init");
    display_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Display Init");
    rcore_buttons_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Buttons Init");
    rtc_init();
    rcore_ambient_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Ambiance Init");
    rcore_backlight_init();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "Backlight Init");
    platform_init_late();
    rcore_watchdog_init_late();
    KERN_LOG("init", APP_LOG_LEVEL_INFO, "watchdog is ticking");
}

/*
 * FreeRTOS code below. deals with mem errors and tick
 */

void vApplicationTickHook(void) {
}

/* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created.  It is also called by various parts of the
   demo application.  If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook(void) {
    KERN_LOG("malloc", APP_LOG_LEVEL_ERROR, "Malloc Failed!");
    taskDISABLE_INTERRUPTS();
    for(;;);
}

/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
   task.  It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()).  If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
void vApplicationIdleHook(void) {
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
        configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
        function is called if a stack overflow is detected. */
    KERN_LOG("init", APP_LOG_LEVEL_ERROR, "Stack Overflow!");
    taskDISABLE_INTERRUPTS();
    for(;;);
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
/*
 * 
 *           Joshua 
 * The below works on the device as well as qemu.
 * The stack data size on arm is 32 bit, so setting to a uint type causes weird
 * pointer issues in freertos.
 * StackType_t is platform independant, and is sized to a frame, pointer. in our case 32 bit.
 * That's also why sizeof(_idle_stack) is 4 times out. Task size is task stack entries * sizeof(int)
 * 
 * Ginge
 * 
 */
static StackType_t _idle_stack[250];
static StaticTask_t _idle_tcb;
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &_idle_tcb;
    *ppxIdleTaskStackBuffer = _idle_stack;
    *pulIdleTaskStackSize = sizeof(_idle_stack) / sizeof(_idle_stack[0]);
}

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
static StackType_t _timer_stack[100];
static StaticTask_t _timer_tcb;
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &_timer_tcb;
    *ppxTimerTaskStackBuffer = _timer_stack;
    *pulTimerTaskStackSize = sizeof(_timer_stack) / sizeof(_timer_stack[0]);
}
