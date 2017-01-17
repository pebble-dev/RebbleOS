#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "stdio.h"
#include "stm32f4xx_usart.h"
#include "buttons.h"
#include "display.h"
#include "vibrate.h"
#include "rtc.h"


//#define printf(fmt, ...) (0)


void init_USART3(void);
void init_USART8(void);
void init_GPIO(void);
void init_hardware(void);

extern buttons_t buttons;


void CheckButton_Timer(void*); // probably move to an interrupt at some point
void CheckTaskWatchDog (void *pvParameters);

int main(void)
{
    SystemInit();

    // not done in SystemInit, it did something weird.
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);

    // set the default pri groups for the interrupts
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    init_hardware();
    // it needs a gentle reminder
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // ginge: the below may only apply to non-snowy
    // joshua - Yesterday at 8:42 PM
    // I recommend setting RTCbackup[0] 0x20000 every time you boot
    // that'll force kick you into PRF on teh next time you enter the bootloader

    // Dump clocks
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    printf("c     : %d", SystemCoreClock);
    printf("SYSCLK: %d\n", RCC_Clocks.SYSCLK_Frequency);
    printf("CFGR  : %x\n", RCC->PLLCFGR);
    printf("Relocating NVIC to 0x08004000\n");

    xTaskCreate(
        CheckTaskWatchDog,                 /* Function pointer */
        "WWDGTask",                          /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 2UL,           /* Task priority*/
        NULL);

    printf("Tasks Created!\n");
    
    printf("FreeRTOS Ginge v0.0.0.2 Codename 'Piddle'\n");
    vTaskStartScheduler();  // should never return
    for (;;);
}

void CheckTaskWatchDog (void *pvParameters)
{
    while(1)
    {
        IWDG_ReloadCounter();
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}



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

void init_hardware(void)
{
    init_USART8(); // smartstrap debugging
    init_USART3(); // general debugging
    printf("USART3 init\n");
    init_GPIO();
    printf("GPIOs init\n");
    power_init();
    vibrate_init();
    display_init();
    buttons_init();
//     rtc_init();
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
void init_USART3(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART3, &USART_InitStruct);
  USART_Cmd(USART3, ENABLE);
}

void init_USART8(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_UART8);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_UART8);

  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(UART8, &USART_InitStruct);
  USART_Cmd(UART8, ENABLE);
}


/**
 * Init HW
 */
void init_GPIO()
{
    // USART1
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    // Init PortB
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        
    // Init Buttons and display bank
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    
    // for vibrate
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
}
