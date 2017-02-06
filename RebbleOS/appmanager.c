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
#include "FreeRTOS.h"
#include "platform.h"
#include "stdio.h"
#include "task.h"
#include "buttons.h"
#include "vibrate.h"
#include "semphr.h"
#include "display.h"
#include "rebbleos.h"

static TaskHandle_t xAppTask;
void vAppTask(void *pvParameters);

void appmanager_init(void)
{
    xTaskCreate(vAppTask, "App", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &xAppTask);
    printf("App Task Created!\n");
}

void appmanager_begin_suspend(void)
{
    // send a signal to the app thread and tell the app to sleep
    vTaskSuspend(xAppTask);
}

void appmanager_resume(void)
{
    // app must give us a frame here
    app_resumed();
    vTaskResume(xAppTask);
}

/*
 * The main task for the button click
 */
void vAppTask(void *pvParameters)
{   
    //start the app
    app_init();
    
    for( ;; )
    {
        app_main();
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}
