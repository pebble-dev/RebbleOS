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
/*
 * MODULE TODO
 * 
 * Button Multi click support
 * buttonholder -> clickrecognizer convert / typedef
 * Theres a couple of bytes of ram to be shaved
 * 
 * review task priorities
 */
#include "rebbleos.h"

static TaskHandle_t xButtonDebounceTask;
static xQueueHandle xButtonQueue;
ButtonMessage button_message;

uint8_t last_press; // ugh

void vButtonTask(void *pvParameters);
void vButtonDebounceTask(void *pvParameters);

void button_update(ButtonId button_id, uint8_t press);
void button_released(ButtonHolder *button);
uint8_t button_check_time(void);
void button_send_app_click(void *callback, void *recognizer, void *context);

ButtonHolder *button_holders[NUM_BUTTONS];

static void button_isr(hw_button_t button_id);

/*
 * Start the button processor
 */
void buttons_init(void)
{
    hw_button_init();
    
    xTaskCreate(vButtonTask, "Button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5UL, NULL);
    xTaskCreate(vButtonDebounceTask, "Debounce", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5UL, &xButtonDebounceTask);
    
    xButtonQueue = xQueueCreate(5, sizeof(uint8_t));
    
    printf("Button Task Created!\n");
    hw_button_set_isr(button_isr);
        
    // Initialise the button click configs
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        button_add_click_config(i, (ClickConfig) {});
    }
}

/*
 * Callback function for the button ISR in hardware.
 */
void button_isr(hw_button_t /* which is definitionally the same as a ButtonID */ button_id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    last_press = button_id;

    vTaskNotifyGiveFromISR(xButtonDebounceTask, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
 * Add a new click handler struct
 */
ButtonHolder *button_add_click_config(ButtonId button_id, ClickConfig click_config)
{
    if (button_id >= NUM_BUTTONS)
        return NULL;
    
    ButtonHolder *button_holder = calloc(1, sizeof(ButtonHolder));

    if (!button_holder) // could not malloc
        return NULL;

    button_holder->click_config = click_config;
    button_holder->button_id = button_id;

    button_holders[button_id] = button_holder;
    
    return button_holder;
}

/*
 * Called up button press or releases after debouncing has occured
 */
void button_update(ButtonId button_id, uint8_t press)
{
    ButtonHolder *button = button_holders[button_id];
    
    // go through the buttons and find any that match
    if (press)
    {
        button->press_time = xTaskGetTickCount();
        button->repeat_time = button->press_time;
            
        // send the raw keypress
        if (button->click_config.raw.down_handler)
        {
            button_send_app_click(button->click_config.raw.down_handler,
                NULL, // TODO
                button->click_config.context);
        }
        button->state = BUTTON_STATE_PRESSED;
    }
    else
    {
        // the button was not pressed at the time we got the message. release it.
        button_released(button);
    }
}

/*
 * release the key. decides which callback to call depending on click settings
 * i.e. will call short click release only if its below a long clck time
 */
void button_released(ButtonHolder *button)
{
    uint32_t now = xTaskGetTickCount();
    uint32_t delta_ms = portTICK_PERIOD_MS * (now - button->press_time);
    
    // we are releasing and it has a click handler
    // if we have long click, we are below the trigger threshold
    if (button->click_config.click.handler ||
        (button->click_config.click.handler && 
        button->click_config.long_click.handler &&
        delta_ms < button->click_config.long_click.delay_ms))
    {
        button_send_app_click(button->click_config.click.handler,
                NULL, // TODO
                button->click_config.context);
    }
    
    // long press release handler
    if (button->click_config.long_click.release_handler && 
        button->state == BUTTON_STATE_LONG)
    {
        // only trigger the release if we are over the repeat time
        button_send_app_click(button->click_config.long_click.release_handler,
                NULL, // TODO
                button->click_config.context);
    }
    
    // just send the raw
    if (button->click_config.raw.up_handler)
    {
        //button->click_config.raw.up_handler(NULL, NULL);
        button_send_app_click(button->click_config.raw.up_handler,
                NULL, // TODO
                button->click_config.context);
    }
    
    button->press_time = 0;
    button->repeat_time = 0;
    button->state = BUTTON_STATE_RELEASED;
}

/*
 * Each button has a timestamp for when it last performed an action
 * If the button is set to, say, repeat, then we look for timed out
 * buttons and trigger the relevant action.
 */
uint8_t button_check_time(void)
{
    uint32_t now = xTaskGetTickCount();
    ButtonHolder *button;
    uint8_t pressed, any_pressed = 0;    

    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        button = button_holders[i];
        pressed = button_pressed(i);
                
        if (!pressed)
        {
            continue;
        }
        else
        {
            any_pressed = 1;
        }
        
        // button is still pressed
        // we have a click and a repeat
        if (button->click_config.click.handler && button->repeat_time > 0 && 
            button->click_config.click.repeat_interval_ms > 0)
        {
            // and the time of the last repeat was < now
            if (now > button->repeat_time + (button->click_config.click.repeat_interval_ms / portTICK_PERIOD_MS))
            {
                button->state = BUTTON_STATE_REPEATING;
                button_send_app_click(button->click_config.click.handler,
                        NULL, // TODO
                        button->click_config.context);
                
                // reset the time
                button->repeat_time = now;
            }
        }
        
        // button pressed, and we just blasted past the long press time
        if (button->click_config.long_click.handler && button->press_time > 0 && 
            button->click_config.long_click.delay_ms > 0)
        {
            if (now > button->press_time + (button->click_config.long_click.delay_ms / portTICK_PERIOD_MS))
            {
                button->state = BUTTON_STATE_LONG;
                button_send_app_click(button->click_config.long_click.handler,
                        NULL, // TODO
                        button->click_config.context);
                
                // stop further processing
                button->press_time = 0;
                button->repeat_time = 0;
            }
        }
    }
    
    return any_pressed;
}

/*
 * Take the incoming button presses from the debounced input, and process them
 * also take care of looping around the button press checker
 * The main loop will only halt into a permanent sleep once
 * all buttons are released. Until that time, it will loop in n ms intervals
 */
void vButtonTask(void *pvParameters)
{
    uint8_t data;
    
    TickType_t time_increment = portMAX_DELAY;
           
    for( ;; )
    {
        if (xQueueReceive(xButtonQueue, &data, time_increment))
        {           
            button_update(data, button_pressed(data));
        }
        
        time_increment = button_check_time();

        if (time_increment == 0)
            time_increment = portMAX_DELAY;
        else
            time_increment = pdMS_TO_TICKS(30);
    }
}

/*
 * Debounce the button
 */
void vButtonDebounceTask(void *pvParameters)
{
    for( ;; )
    {
        // sleep forever waiting for something on the interrupts to wake us
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // tell the main worker we have something
        xQueueSendToBack(xButtonQueue, &last_press, (TickType_t)100);

        // because we are using a notification on the task, any further triggers while
        // the buttons are bouncing are ignored while we are sleeping
        // so lets go ahead and have a nap                           
        vTaskDelay(butDEBOUNCE_DELAY);
        
        // one last cleardown of our notification just to be sure
        ulTaskNotifyTake(pdTRUE, 0);
    }
}


/*
 * Convenience function to send a button click to the main application processor
 */
void button_send_app_click(void *callback, void *recognizer, void *context)
{   
    button_message.callback = callback;
    button_message.clickref = recognizer; // TODO
    button_message.context  = context;

    backlight_on(100, 3000);
    
    appmanager_post_button_message(&button_message);
}



/*
 * Check if a button is pressed
 */
uint8_t button_pressed(ButtonId button_id)
{   
    return hw_button_pressed(button_id);
}

/*
 * These are the subscription handlers for the button inputs and their various settings
 */

void button_single_click_subscribe(ButtonId button_id, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = button_holders[button_id]; // get the button
    holder->click_config.click.handler = handler;
    holder->click_config.click.repeat_interval_ms = 0;
}

void button_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = button_holders[button_id]; // get the button
    holder->click_config.click.handler = handler;
    holder->click_config.click.repeat_interval_ms = repeat_interval_ms;
}

void button_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
//     ButtonHolder *holder = button_holders[button_id]; // get the button
}

void button_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = button_holders[button_id]; // get the button
    holder->click_config.long_click.handler = down_handler;
    holder->click_config.long_click.release_handler = up_handler;
    holder->click_config.long_click.delay_ms = delay_ms;
}

void button_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = button_holders[button_id]; // get the button
    holder->click_config.raw.up_handler = up_handler;
    holder->click_config.raw.down_handler = down_handler;
}

void button_unsubscribe_all(void)
{
    for(uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        ButtonHolder *holder = button_holders[i]; // get the button
        memset(holder, 0, sizeof(ClickConfig));
    }
}
