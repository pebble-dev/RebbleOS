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
#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "display.h"
#include "snowy_display.h"
#include "task.h"
#include "semphr.h"
#include "logo.h"

static TaskHandle_t xDisplayCommandTask;
static xQueueHandle xQueue;

static UG_GUI gui;

extern display_t display;

/*
 * Start the display driver and tasks. Show splash
 */
void display_init(void)
{   
    // init variables
    display.BacklightEnabled = 0;
    display.Brightness = 0;
    display.PowerOn = 0;
    display.State = DISPLAY_STATE_BOOTING;
    
    printf("Display Init\n");
    
    // initialise device specific display
    hw_display_init();
    hw_backlight_init();
    
    hw_display_start();
        
    // set up the RTOS tasks
    xTaskCreate(vDisplayCommandTask, "Display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2UL, &xDisplayCommandTask);    
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
        
    printf("Display Tasks Created!\n");
    
    // turn on the LCD draw
    display.DisplayMode = DISPLAY_MODE_BOOTLOADER;   
    
    init_gui();
    
    display_cmd(DISPLAY_CMD_DRAW, NULL);
}

/*
 * When the display driver has responded to something
 * 1) frame frame accepted
 * 2) frame completed
 * We get notified here. We can now let the prcessing complete or continue
 */
void display_done_ISR(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the task that the transmission is complete.
    //vTaskNotifyGiveFromISR(xDisplayISRTask, &xHigherPriorityTaskWoken);
    vTaskNotifyGiveFromISR(xDisplayCommandTask, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


/*
 * Brutally and forcefully reset the display. This will
 * reset, but it will leave you dead in the water. Make sure to init.
 * or, instead, use init
 */
void display_reset(uint8_t enabled)
{
    hw_display_reset();
}

/*
 * We can command the screen on and off. but only in bootloader mode. hrmph
 */
void display_on()
{    
//    display.State = DISPLAY_STATE_TURNING_ON;
}

/*
 * Begin rendering a frame from the framebuffer into the display
 */
void display_start_frame()
{
    display.State = DISPLAY_STATE_FRAME_INIT;
    
    hw_display_start_frame();
}

/*
 * Set the backlight. At the moment this is scaled to be 4000 - mid brightness
 */
void backlight_set(uint16_t brightness)
{
    display.Brightness = brightness;
    
    // set the display pwm value
    hw_backlight_set(brightness);
}

/*
 * Given a frame of data, convert it and draw it to the display
 */
void display_logo(char *frameData)
{
    for(uint16_t i = 0; i < 24192; i++)
    {
        frameData[i] = rebbleOS[i];
    }
    scanline_convert_buffer();
    return;
}

/*
 * Display a checkerboard picture for testing
 */
uint16_t display_checkerboard(char *frameData, uint8_t invert)
{
    int gridx = 0;
    int gridy = 0;
    int count = 0;

    uint8_t forCol = RED;
    uint8_t backCol = GREEN;
    
    // display a checkboard    
    if (invert)
    {
        forCol = GREEN;
        backCol = RED;
    }
    
    for (int x = 0; x < display.NumCols; x++) {
        gridy = 0;
        if ((x % 21) == 0)
            gridx++;
        
        for (int y = 0; y < display.NumRows; y++) {
            if ((y % 21) == 0)
                gridy++;
            
            if (gridy%2 == 0)
            {
                if (gridx%2 == 0)
                    frameData[count] = forCol;
                    //display_SPI6_send(RED);
                else
                    frameData[count] = backCol;
                    //display_SPI6_send(GREEN);
            }
            else
            {
                if (gridx%2 == 0)    
                    frameData[count] = backCol;
                    //display_SPI6_send(GREEN);
                else
                    //display_SPI6_send(RED);
                    frameData[count] = forCol;
            }
            
            
            count++;
        }
    }
    return count;
}


/*
 * Request a command from the display driver. 
 * Such as DISPLAY_CMD_DRAW
 */
void display_cmd(uint8_t cmd, char *data)
{
    xQueueSendToBack(xQueue, &cmd, 0);
}

/*
 * Main task processing for the display. Manages locking
 * state machine control and command management
 */
void vDisplayCommandTask(void *pvParameters)
{
    uint8_t data;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    display.State = DISPLAY_STATE_BOOTING;
    uint32_t ulNotificationValue;
    char buf[30];
    
    while(1)
    {
        if (display.State != DISPLAY_STATE_IDLE &&
            display.State != DISPLAY_STATE_BOOTING)
        {
            vTaskDelay(5 / portTICK_RATE_MS);
            continue;
        }
        
        // commands to be exectuted are send to this queue and processed
        // one at a time
        if (xQueueReceive(xQueue, &data, xMaxBlockTime))
        {
            switch(data)
            {
                case DISPLAY_CMD_DRAW:
                    // all we are responsible for is starting a frame draw
                    display_start_frame();
                    break;
            }
        }
        else
        {
            // nothing emerged from the buffer
            hw_get_time_str(buf);
            UG_ConsolePutString(buf);

            //UG_Update();
            display_start_frame();
            //display_cmd(DISPLAY_CMD_DRAW, 0);
        }        
    }
}


// GUI related

/*
 * Initialise the uGUI component and start the draw
 */
int init_gui(void)
{   
    /* Configure uGUI */
    UG_Init(&gui, scanline_rgb888pixel_to_frambuffer, display.NumCols, display.NumRows);

    /* Draw text with uGUI */
    UG_FontSelect(&FONT_8X14);
    UG_ConsoleSetArea(0, 0, display.NumCols-1, display.NumRows-1);
    UG_ConsoleSetBackcolor(C_BLACK);
    UG_ConsoleSetForecolor(C_GREEN);
    UG_ConsolePutString("RebbleOS...\n");
    UG_ConsoleSetForecolor(C_GREEN);
    UG_ConsolePutString("Version 0.00001\n");
    UG_ConsoleSetForecolor(C_BLUE);
    UG_ConsolePutString("Condition: Mauve\n");
    UG_ConsoleSetForecolor(C_RED);

    //menu_draw_list();
    return 0;
}
