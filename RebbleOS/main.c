/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"
#include "rebbleos.h"

void CheckTaskWatchDog (void *pvParameters);

int main(void)
{
    SystemInit();

    // not done in SystemInit, it did something weird.
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);

    // set the default pri groups for the interrupts
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    hardware_init();
    // it needs a gentle reminder
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // ginge: the below may only apply to non-snowy
    // joshua - Yesterday at 8:42 PM
    // I recommend setting RTCbackup[0] 0x20000 every time you boot
    // that'll force kick you into PRF on teh next time you enter the bootloader

    // Dump clocks
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    printf("c     : %lu", SystemCoreClock);
    printf("SYSCLK: %lu\n", RCC_Clocks.SYSCLK_Frequency);
    printf("CFGR  : %lx\n", RCC->PLLCFGR);
    printf("Relocating NVIC to 0x08004000\n");

    xTaskCreate(
        CheckTaskWatchDog,                 /* Function pointer */
        "WWDGTask",                          /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 5UL,           /* Task priority*/
        NULL);

    rebbleos_init();
    
    printf("RebbleOS (Ginge) v0.0.0.0\n");
    
    vTaskStartScheduler();  // should never return
    for (;;);
}


/*
 * Initialise the system watchdog timer
 */
void watchdog_init()
{
    hw_watchdog_init();
    watchdog_reset();
    //printf("Watchdog Initialised\n");
}

/*
 * Reset the watchdog timer
 */
void watchdog_reset(void)
{
    hw_watchdog_reset();
}

/*
 * A task to periodically reset the watchdog timer
 */
void CheckTaskWatchDog(void *pvParameters)
{
    while(1)
    {
        watchdog_reset();
        vTaskDelay(WATCHDOG_RESET_MS / portTICK_RATE_MS);
    }
}

/*
 * Initialise the whole Rebble platform
 */
void hardware_init(void)
{
    platform_init();
    debug_init();
    printf("debug init\n");
    watchdog_init();
    printf("watchdog init\n");
    power_init();
    printf("power init\n");
    vibrate_init();
    printf("vibro init init\n");
    display_init();
    printf("display init\n");
    buttons_init();
    printf("buttons init\n");
    rtc_init();
    ambient_init();
    printf("ambiance init\n");
    backlight_init();
    printf("bl init\n");
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
  taskDISABLE_INTERRUPTS();
  for(;;);
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {

}

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {

}
