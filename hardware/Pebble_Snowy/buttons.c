#include "stm32f4xx.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "buttons.h"
#include "semphr.h"

void buttons_init_intn(void);

void EXTI4_IRQHandler_isr(void);

static TaskHandle_t xButtonTask;
#define butDEBOUNCE_DELAY	( 200 / portTICK_RATE_MS )

buttons_t buttons = {
    .Back   = { GPIO_Pin_4, GPIOG },
    .Up     = { GPIO_Pin_3, GPIOG },
    .Select = { GPIO_Pin_1, GPIOG },
    .Down   = { GPIO_Pin_2, GPIOG }
};

void buttons_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure2;
    
    // For snowy we can shortcut the port init. For other boards each bank port will need a declaration for each unique bank
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure2.GPIO_Pin =  buttons.Back.Pin | buttons.Select.Pin | buttons.Up.Pin | buttons.Down.Pin;
    GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(buttons.Back.Port, &GPIO_InitStructure2);
    
    xTaskCreate(vButtonTask, "Button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL, &xButtonTask); 
    printf("Button Task Created!\n");
    buttons_init_intn();
}

void buttons_init_intn(void) {
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Tell system that you will use PD0 for EXTI_Line0 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource4);
    
    /* Button is connected to EXTI_Linen */
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 13;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
}

/* Set interrupt handlers */
void EXTI4_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {       
        EXTI4_IRQHandler_isr();
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI4_IRQHandler_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR( xButtonTask, &xHigherPriorityTaskWoken );
    //printf("ISR EXTI\n");

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

uint8_t button_is_pressed(button_t *button) {
    int btn = GPIO_ReadInputDataBit(button->Port, button->Pin);
    
    return btn;
}


void vButtonTask(void *pvParameters)
{
    /* Ensure the semaphore is created before it gets used. */
    //xButtonSemaphore = xSemaphoreCreateBinary();
    uint32_t ulNotificationValue;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);

    for( ;; )
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);

        if( ulNotificationValue == 1 )
        {
            if (button_is_pressed(&buttons.Back))
            {
                printf("Back Button Released\n");
            }
            else 
            {
                printf("Back Button Pressed\n");
            }
                
        }
        else
        {
            //printf("Timeout!\n");
        }

        
        /* Wait a short time then clear any pending button pushes as a crude
        method of debouncing the switch.  xSemaphoreTake() uses a block time of
        zero this time so it returns immediately rather than waiting for the
        interrupt to occur. */
        vTaskDelay( butDEBOUNCE_DELAY );
    }
}
