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
#include "rebbleos.h"

static TaskHandle_t xButtonTask;
button_t *lastPress;
void vButtonTask(void *pvParameters);

extern buttons_t buttons;

/*
 * Start the button processor
 */
void buttons_init(void)
{
    hw_buttons_init();
    
    xTaskCreate(vButtonTask, "Button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5UL, &xButtonTask);
    printf("Button Task Created!\n");
}

/*
 * Callback function for the button press code.
 */
void button_isr(button_t *button)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    lastPress = button;
    printf("Button ISR\n");
    printf("Button ISR %d\n", GPIO_ReadInputDataBit(button->Port, button->Pin));


    vTaskNotifyGiveFromISR(xButtonTask, &xHigherPriorityTaskWoken);   

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
 * Check if a button is pressed
 */
uint8_t button_pressed(button_t *button)
{   
    return hw_button_pressed(button);
}

/*
 * The main task for the button click
 */
void vButtonTask(void *pvParameters)
{
    uint32_t ulNotificationValue;
    
    for( ;; )
    {
        uint8_t dir = 0;
        uint8_t mode = BTN_SELECT_PRESS;
        
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        //if(ulNotificationValue == 1)
        {           
            if (button_pressed(lastPress))
            {
                printf("Button Released\n");
                dir = 0;
            }
            else
            {
                printf("Button Pressed\n");
                dir = 1;
            }
            
            // multiple buttons can be pressed ;)
            if (lastPress == &buttons.Up)
            {
                printf("UP\n");
                if (dir == 1)
                {
                    mode = BTN_UP_PRESS;
                }
            }
            if (lastPress == &buttons.Down)
            {
                printf("DN\n");
                if (dir == 1)
                {
                    mode = BTN_DOWN_PRESS;
                }

            }
            if (lastPress == &buttons.Select)
            {
                printf("SL\n");
                if (dir == 1)
                {
                    mode = BTN_SELECT_PRESS;
                }
            }
            if (lastPress == &buttons.Back)
            {
                printf("BK\n");
                if (dir == 1)
                {
                    mode = BTN_BACK_PRESS;
                }
               
            }
            
            if (dir == 1)
                gui_command(mode);
                    
            vTaskDelay(butDEBOUNCE_DELAY);
            ulTaskNotifyTake(pdTRUE, 0);
        }
//         else
//         {
//             printf("Timeout!\n");
//         }
    }
}
