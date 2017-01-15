#include "stm32f4xx.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "buttons.h"
#include "vibrate.h"
#include "semphr.h"

void buttons_init_intn(uint32_t EXTIport, uint32_t EXTIline, uint32_t pinSource, uint32_t EXTI_irq);
void EXTI4_IRQHandler_isr(void);

static TaskHandle_t xButtonTask;
button_t *lastPress;

#define butDEBOUNCE_DELAY	( 10 / portTICK_RATE_MS )

buttons_t buttons = {
    .Back   = { GPIO_Pin_4, GPIOG },
    .Up     = { GPIO_Pin_3, GPIOG },
    .Select = { GPIO_Pin_1, GPIOG },
    .Down   = { GPIO_Pin_2, GPIOG }
};

void buttons_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure2;
    
    // For snowy we can shortcut the port init. For other boards each bank port will need a declaration for each unique bank
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure2.GPIO_Pin =  buttons.Back.Pin | buttons.Select.Pin | buttons.Up.Pin | buttons.Down.Pin;
    GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(buttons.Back.Port, &GPIO_InitStructure2);
    
    xTaskCreate(vButtonTask, "Button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL, &xButtonTask);
    
    printf("Button Task Created!\n");
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line4, EXTI_PinSource4, EXTI4_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line3, EXTI_PinSource3, EXTI3_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line2, EXTI_PinSource2, EXTI2_IRQn);
    buttons_init_intn(EXTI_PortSourceGPIOG, EXTI_Line1, EXTI_PinSource1, EXTI1_IRQn);
}

// TODO make generic
void buttons_init_intn(uint32_t EXTIport, uint32_t EXTIline, uint32_t pinSource, uint32_t EXTI_irq)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    SYSCFG_EXTILineConfig(EXTIport, pinSource);
    
    EXTI_InitStruct.EXTI_Line = EXTIline;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&EXTI_InitStruct);
 
    // Add IRQ vector to NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI_irq;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/* Set interrupt handlers */
void EXTI4_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {       
        button_isr(&buttons.Back);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI3_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {       
        button_isr(&buttons.Up);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI2_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {       
        button_isr(&buttons.Down);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI1_IRQHandler(void)
{
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {       
        button_isr(&buttons.Select);
       
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

// Quirk of ARM/RTOS. Do processing in another function other it won't execute
void button_isr(button_t *button)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    lastPress = button;
    vTaskNotifyGiveFromISR(xButtonTask, &xHigherPriorityTaskWoken);
    

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

uint8_t button_is_pressed(button_t *button)
{
    int btn = GPIO_ReadInputDataBit(button->Port, button->Pin);
    
    return btn;
}
GPIO_TypeDef *port = GPIOA_BASE;
uint8_t pin = 0;

GPIO_TypeDef *setNextPort(GPIO_TypeDef *port);

void setNextPin(void)
{
    pin++;
    
    if (pin >= 16)
    {
        port = setNextPort(port);
        pin = 0;
    }
    
    // TODO set as output
    
    GPIO_SetBits(port, 1 << pin);
    printf("SET Port %d Pin %d\n", port, pin);
}

GPIO_TypeDef *setNextPort(GPIO_TypeDef *port)
{
    GPIO_TypeDef *nport = port + 0x400;
    
    if (nport >= GPIOA_BASE + 0x1800)
        nport = GPIOA_BASE;
    
    return nport;
}


void vButtonTask(void *pvParameters)
{
    uint32_t ulNotificationValue;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    int16_t scene = 9999;
    
    for( ;; )
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        uint8_t dir = 0;
        
        if( ulNotificationValue == 1 )
        {
            if (button_is_pressed(lastPress))
            {
                printf("Button Released\n");
                dir = 0;
            }
            else 
            {
                printf("Button Pressed\n");
                dir = 1;
            }
             
            if (lastPress == &buttons.Up)
            {
                printf("UP\n");
                if (dir == 1)
                {
                    //vibrate_enable(1);
                    //snowy_display_drawscene(3);
                }
            }
            if (lastPress == &buttons.Down)
            {
                printf("DN\n");
                if (dir == 1)
                {
                    //vibrate_enable(0);
                    //display_drawscene(2);
                    snowy_display_full_init();
                }
            }
            if (lastPress == &buttons.Select)
            {
                printf("SL\n");
                if (dir == 1)
                {
                    scene+=500;
                    if (scene >= 9999)
                        scene = 99;

                    printf("scene %d\n", scene);
                   
                    //display_drawscene(scene);
                    backlight_set(scene);
                    
                }
            }
            if (lastPress == &buttons.Back)
            {
                printf("BK\n");
                if (dir == 1)
                {
                    scene-=500;
                    if (scene <= 0)
                        scene = 9999;

                    printf("scene %d\n", scene);
                    //display_drawscene(scene);
                    backlight_set(scene);
                }
            }
        }
        else
        {
            printf("Timeout!\n");
        }

        
        /* Wait a short time then clear any pending button pushes as a crude
        method of debouncing the switch.  xSemaphoreTake() uses a block time of
        zero this time so it returns immediately rather than waiting for the
        interrupt to occur. */
        vTaskDelay(butDEBOUNCE_DELAY);
    }
}
