#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "stdio.h"
#include "stm32f4xx_usart.h"
#include "buttons.h"

void init_USART3(void);
void init_GPIO(void);

void CheckButton_Timer(void*); // probably move to an interrupt at some point

int main(void) {
    SystemInit();

    // not done in SystemInit, it did something weird.
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    init_USART3();
    printf("USART3 init\n");
    printf("Relocating NVIC to 0x08004000\n");
  
    init_GPIO();
    printf("GPIOs init\n");

    xTaskCreate(
        CheckButton_Timer,                 /* Function pointer */
        "Task1",                          /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 2UL,           /* Task priority*/
        NULL);

    printf("Task Created!\n");
    
    printf("Turn on the display!\n");
    //display_init();

    printf("FreeRTOS Ginge v0.0.0.1 Codename 'Piddle'\n");
    vTaskStartScheduler();  // should never return
    for (;;);
}



/**
 
 */
void CheckButton_Timer(void *pvParameters){
  printf("Task ctor\n");
  int lastback = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Back);
  int lastup = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Up);
  int lastdown = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Down);
  int lastsel = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Select);
  
  while (1) {
    GPIO_ToggleBits(GPIOD, GPIO_Pin_12);

    // Hacky button testing in a polling loop
    // TODO move to buttons.c
    // TODO really crappy code. sort it
    // uses totally crapy debounce. its slow and glitchy. sort.
    int btn = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Back);
    if (btn != lastback) {
        if (btn)
            printf("Back Up!\n");    //back GPIO_Pin_4
        else
            printf("Back Down!\n");    //back GPIO_Pin_4
    }
    lastback = btn;

    btn = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Up);
    if (btn != lastup) {
        if (btn)
            printf("Up Up!\n");    //back GPIO_Pin_4
        else
            printf("Up Down!\n");    //back GPIO_Pin_4
    }
    lastup = btn;
    
    btn = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Down);
    if (btn != lastdown) {
        if (btn)
            printf("Down Up!\n");    //back GPIO_Pin_4
        else
            printf("Down Down!\n");    //back GPIO_Pin_4
    }
    lastdown = btn;
    
    btn = GPIO_ReadInputDataBit(BUTTON_GPIO, buttons.Select);
    if (btn != lastsel) {
        if (btn)
            printf("Select Up!\n");    //back GPIO_Pin_4
        else
            printf("Select Down!\n");    //back GPIO_Pin_4
    }
    lastsel = btn;
        
    /*
    Delay for a period of time. vTaskDelay() places the task into
    the Blocked state until the period has expired.
    The delay period is spacified in 'ticks'. We can convert
    yhis in milisecond with the constant portTICK_RATE_MS.
    */
    vTaskDelay(80 / portTICK_RATE_MS);
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

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
void init_USART3(void) {
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;

  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART3, &USART_InitStruct);
  USART_Cmd(USART3, ENABLE);
}



/**
 * Init HW
 */
void init_GPIO()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure2;
  
  // Init Vibro? nah, no idea yet.
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
    
  // Init Buttons
  // TODO move this to buttons.c
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
  GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure2.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOG, &GPIO_InitStructure2);
}
