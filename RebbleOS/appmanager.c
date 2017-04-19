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

#include <stdlib.h>
#include "rebbleos.h"
#include "appmanager.h"
#include "systemapp.h"

static TaskHandle_t xAppTask;
void vAppTask(void *pvParameters);
static xQueueHandle xAppMessageQueue;
void back_long_click_handler(ClickRecognizerRef recognizer, void *context);
void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context);

App *running_app;

// TODO cheesy
#define APP_COUNT 3
App apps[APP_COUNT];

// simple doesn't have an include, so cheekily forward declare here
void simple_main(void);
void nivz_main(void);


void appmanager_init(void)
{
    // create our apps. we will want an app registry at some point
    strcpy(apps[0].name, "System");
    apps[0].main = (void*)systemapp_main;
    apps[0].type = APP_TYPE_SYSTEM;
    
    strcpy(apps[1].name, "Simple");
    apps[1].main = (void*)simple_main;
    apps[1].type = APP_TYPE_FACE;
    
    strcpy(apps[2].name, "NiVZ");
    apps[2].main = (void*)nivz_main;
    apps[2].type = APP_TYPE_FACE;
    
   
    xTaskCreate(vAppTask, "App", configMINIMAL_STACK_SIZE * 20, NULL, tskIDLE_PRIORITY + 5UL, &xAppTask);
    xAppMessageQueue = xQueueCreate(5, sizeof(struct AppMessage));
    printf("App Task Created!\n");
}

void appmanager_app_start(char *name)
{
    // kill the current app
    appmanager_app_quit();
    
    // TODO reset clicks
    
    // app_bin_load_to_ram();
    // app_bin_load_execute();
    
    // load the next app by name
    // TODO do some fancy loader
    for (uint8_t i = 0; i < APP_COUNT; i++)
    {
        if (!strcmp(name, apps[i].name))
        {
            // it's the one
            running_app = &apps[i];
            break;
        }
    }
    
    // we are setup now for main.
    // signal go to the thread
    xTaskNotifyGive(xAppTask);
}

void appmanager_app_quit(void)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_QUIT,
        .payload = NULL
    };
    xQueueSendToBack(xAppMessageQueue, &am, (TickType_t)10);
}

void appmanager_post_button_message(ButtonMessage *bmessage)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_BUTTON,
        .payload = (void *)bmessage
    };
    xQueueSendToBack(xAppMessageQueue, &am, (TickType_t)10);
}

void appmanager_post_tick_message(TickMessage *tmessage, BaseType_t *pxHigherPri)
{
    AppMessage am = (AppMessage) {
        .message_type_id = APP_TICK,
        .payload = (void *)tmessage
    };
    // Note the from ISR. The tic comes direct to the app event handler
    xQueueSendToBackFromISR(xAppMessageQueue, &am, pxHigherPri);
}

void app_event_loop(void)
{
    uint32_t xMaxBlockTime = 1000 / portTICK_RATE_MS;
    AppMessage data;
    
    // we assume they are configured now
    rbl_window_load_proc();
    rbl_window_load_click_config();
    
    // Install our own handler to hijack the long back press
    //window_long_click_subscribe(BUTTON_ID_BACK, 1100, back_long_click_handler, back_long_click_release_handler);
    
    // TODO move to using local running_app variable to make atomic
    window_single_click_subscribe(BUTTON_ID_SELECT, app_select_single_click_handler);
    // hook the return from menu if we are a system app
    if (running_app->type == APP_TYPE_SYSTEM)
        window_single_click_subscribe(BUTTON_ID_BACK, back_long_click_handler);
    
    // redraw
    window_dirty(true);
    
    // block forever
    for ( ;; )
    {
        // we are inside the apps main loop event handler now
        if (xQueueReceive(xAppMessageQueue, &data, xMaxBlockTime))
        {
            if (data.message_type_id == APP_BUTTON)
            {
                // execute the button's callback
                ButtonMessage *message = (ButtonMessage *)data.payload;
                ((ClickHandler)(message->callback))((ClickRecognizerRef)(message->clickref), message->context);
            }
            else if (data.message_type_id == APP_TICK)
            {
                // execute the timers's callback
                TickMessage *message = (TickMessage *)data.payload;
                
                ((TickHandler)(message->callback))(message->tick_time, (TimeUnits)message->tick_units);
            }
            else if (data.message_type_id == APP_QUIT)
            {
                // remove all of the clck handlers
                button_unsubscribe_all();
                // remove the ticktimer service handler and stop it
                rebble_time_service_unsubscribe();
                printf("ev quit\n");
                // app was quit, break out of this loop into the main handler
                break;
            }
        }
    }
    // the app itself will quit now
}

/*
 * The main task for the button click
 */
void vAppTask(void *pvParameters)
{
    
    // set off using system
    appmanager_app_start("System");
    
    
    for( ;; )
    {
        // Sleep waiting for the go signal
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
 
        // clear the queue of any work from the previous app... such as an errant quit
        xQueueReset(xAppMessageQueue);
        
        // start the app. this will block if the app is written to call
        // the main app_event_loop.
        // The main loop work is deferred to the app until it quits
        ((AppMainHandler)(running_app->main))();
        
        // we unblocked. It looks like the app quit
        // around we go again
        
        // vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void back_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(running_app->type)
    {
        case APP_TYPE_FACE:
            printf("TODO: Quiet time\n");
            break;
        case APP_TYPE_SYSTEM:
            // quit the app
            appmanager_app_start("Simple");
            break;
    }
}

void back_long_click_release_handler(ClickRecognizerRef recognizer, void *context)
{
    printf("Long Back Rel\n");
}

void app_select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch(running_app->type)
    {
        case APP_TYPE_FACE:
            appmanager_app_start("System");
            break;
        case APP_TYPE_SYSTEM:
            menu_select();
            break;
    }
}
