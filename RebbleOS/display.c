#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "snowy_display.h"
#include "task.h"
#include "semphr.h"
#include "logo.h"

static TaskHandle_t xDisplayCommandTask;
static TaskHandle_t xDisplayISRTask;
static xQueueHandle xQueue;

static UG_GUI gui;

extern display_t display;

int init_gui(void);

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
    //xTaskCreate(vDisplayISRProcessor, "DispISR", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &xDisplayISRTask); 
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
        
    printf("Display Tasks Created!\n");
    
    // turn on the LCD draw
    display.DisplayMode = DISPLAY_MODE_BOOTLOADER;   
    
    init_gui();
    
    display_cmd(DISPLAY_CMD_DRAW, NULL);
    
//     if (hw_display_start() == 0)
//     {
//         printf("Display Failed!\n");
//     }
}


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


// Pull the reset pin
void display_reset(uint8_t enabled)
{
    hw_display_reset();
}

// command the screen on
void display_on()
{    
//    display.State = DISPLAY_STATE_TURNING_ON;
}

void display_start_frame()
{
    display.State = DISPLAY_STATE_FRAME_INIT;
    
    hw_display_start_frame();
}

void display_send_frame()
{
    display.State = DISPLAY_STATE_FRAME;
    
    hw_display_send_frame();
}

void backlight_set(uint16_t brightness)
{
    display.Brightness = brightness;
    
    // set the display pwm value
    hw_backlight_set(brightness);
}

void display_logo(char *frameData)
{
    for(uint16_t i = 0; i < 24192; i++)
    {
        frameData[i] = rebbleOS[i];
    }
    scanline_convert_buffer();
    return;
}

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

// request a command from the display driver. this will queue up your request and
// process it in order
void display_cmd(uint8_t cmd, char *data)
{
    xQueueSendToBack(xQueue, &cmd, 0);
}

// state machine thread for the display
// keeps track of init, flash and frame management
void vDisplayCommandTask(void *pvParameters)
{
    uint8_t data;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    display.State = DISPLAY_STATE_BOOTING;
    uint32_t ulNotificationValue;
    char buf[30];
    
    while(1)
    {
        // When we are processing a frame, we won't accept any more draw commands
        // until it either completes or times out.
        if (display.State != DISPLAY_STATE_IDLE &&
            display.State != DISPLAY_STATE_BOOTING
        )
        {
            ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(30));
            if(ulNotificationValue == 0)
            {
                printf("ERROR: No ISR Responses in 30ms\n");
                // should probably reset the display here
                display.State = DISPLAY_STATE_IDLE;
                continue;
            }
            else
            {
                // we might have been released for sevel states
                switch(display.State)
                {
                    case DISPLAY_STATE_FRAME_INIT:
                        display_send_frame();
                        break;
                    case DISPLAY_STATE_FRAME:
                        // the fame was drawn. idle now
                        display.State = DISPLAY_STATE_IDLE;
                        break;
                }
            }
            
            display.State = DISPLAY_STATE_IDLE;
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


// GUI related tests

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

    return 0;
}
