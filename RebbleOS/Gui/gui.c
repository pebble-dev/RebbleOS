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
#include "task.h"
#include "semphr.h"
#include "stdio.h"
#include "rebbleos.h"
#include "gui.h"
#include "display.h"
#include "menu.h"

extern display_t display;

void GuiTask (void *pvParameters);
void AnimateTask(void *pvParameters);
static xQueueHandle xAnimQueue;

/*
 * Initialise the uGUI component and start the draw
 */
int gui_init(void)
{   
    /* Configure uGUI */
    UG_Init(&gui, scanline_rgb888pixel_to_frambuffer, display.NumCols, display.NumRows);

    /* Draw text with uGUI */
    UG_FontSelect(&FONT_8X14);
//     UG_ConsoleSetArea(0, 0, display.NumCols-1, display.NumRows-1);
//     UG_ConsoleSetBackcolor(C_BLACK);
//     UG_ConsoleSetForecolor(C_GREEN);
//     UG_ConsolePutString("RebbleOS...\n");
//     UG_ConsoleSetForecolor(C_GREEN);
//     UG_ConsolePutString("Version 0.00001\n");
//     UG_ConsoleSetForecolor(C_BLUE);
//     UG_ConsolePutString("Condition: Mauve\n");
//     UG_ConsoleSetForecolor(C_RED);

    menu_init();
    
    xAnimQueue = xQueueCreate(3, sizeof(uint8_t));
    xGuiQueue = xQueueCreate(10, sizeof(uint8_t));
    
    xTaskCreate(
        GuiTask,                          /* Function pointer */
        "GUI",                            /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 3UL,           /* Task priority*/
        NULL);

    xTaskCreate(
        AnimateTask,                      /* Function pointer */
        "Anim",                           /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 2UL,           /* Task priority*/
        NULL);
        
    printf("Gui tasks init\n");
    

    return 0;
}

void gui_command(uint8_t command)
{
    xQueueSend(xGuiQueue, &command, 0);
}


/*
 * A task to draw the gui and animations
 */
void GuiTask(void *pvParameters)
{
    uint8_t data;
    
    while(1)
    {
        if (xQueueReceive(xGuiQueue, &data, 1000 / portTICK_RATE_MS))  // wait forever for a command
        {
            printf("rcv %d!\n", data);
            switch (data)
            {
                case BTN_SELECT_PRESS:
                    if (system_status.app_mode == SYSTEM_RUNNING_APP)
                    {
                        appmanager_begin_suspend();
                        // we are in app, we need to fade the menu in
                        uint8_t mode = ANIM_RTL;
                        xQueueSendToBack(xAnimQueue, &mode, 0);
                        system_status.app_mode = SYSTEM_IN_MAIN_MENU;
                    }
                    break;
                case BTN_BACK_PRESS:
                    if (system_status.app_mode == SYSTEM_IN_MAIN_MENU)
                    {
                        system_status.app_mode = SYSTEM_RUNNING_APP;
                        appmanager_resume();
                        display_start_frame(0, 0);
                    }
                    break;
                case BTN_UP_PRESS:
                    if (system_status.app_mode == SYSTEM_IN_MAIN_MENU)
                    {
                        menu_up();
                        menu_draw_list(main_menu, 0, 0);
                        display_start_frame(0, 0);
                    }
                    break;
                case BTN_DOWN_PRESS:
                    if (system_status.app_mode == SYSTEM_IN_MAIN_MENU)
                    {
                        menu_down();
                        menu_draw_list(main_menu, 0, 0);
                        display_start_frame(0, 0);
                    }
                    break;                    
            }
            
        }
    }
}

#define GUI_IDLE      0
#define GUI_ANIMATING 1

void AnimateTask(void *pvParameters)
{
    uint8_t data;
    uint8_t wait = 0;
    uint8_t gui_status = GUI_IDLE;
    uint16_t frame = 0;
    
    while(1)
    {
        if (gui_status == GUI_ANIMATING)
        {
            // draw the frame in the position we are at
            wait = 0;
        }
        else
        {
            wait = 1000 / portTICK_RATE_MS;
        }
        
        if (xQueueReceive(xAnimQueue, &data, wait))
        {
            switch(data)
            {
                case ANIM_STOP:
                    break;
                case ANIM_RTL:
                    gui_status = GUI_ANIMATING;
                    menu_draw_list(main_menu, 0, 0);
                    break;
            }
        }
        
        // if we are animating, send the next frame
        // todo etc
        if (gui_status == GUI_ANIMATING)
        {        
            if (frame >= display.NumCols)
            {
                frame = 0;
                gui_status = GUI_IDLE;
                continue;
            }
        
            //UG_Update();
            
            display_start_frame(display.NumCols - frame, 0);

            frame += 10;
        }
            
        //vTaskDelay(10 / portTICK_RATE_MS);
    }
}
