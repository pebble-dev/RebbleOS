#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "snowy_display.h"
#include "task.h"
#include "semphr.h"
#include "ugui.h"
#include "logo.h"

static TaskHandle_t xDisplayCommandTask;
static TaskHandle_t xDisplayISRTask;
static xQueueHandle xQueue;

extern display_t display;

int init_gui(void);

void display_init(void)
{   
    // init variables
    display.BacklightEnabled = 0;
    display.Brightness = 0;
    display.PowerOn = 0;
    display.State = 0;
    
    printf("Display Init\n");
    
    // initialise device specific display
    hw_display_init();
    hw_backlight_init();
        
    // set up the RTOS tasks
    xTaskCreate(vDisplayCommandTask, "Display", configMINIMAL_STACK_SIZE + 150, NULL, tskIDLE_PRIORITY + 2UL, &xDisplayCommandTask);    
    xTaskCreate(vDisplayISRProcessor, "DispISR", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2UL, &xDisplayISRTask); 
    
    xQueue = xQueueCreate( 10, sizeof(uint8_t) );
        
    printf("Display Tasks Created!\n");
    
    // turn on the LCD draw
    display.DisplayMode = DISPLAY_MODE_BOOTLOADER;
    
    hw_display_start();
    
    display_cmd(DISPLAY_CMD_DRAW, NULL);
    
    //init_gui();
    
//     if (hw_display_start() == 0)
//     {
//         printf("Display Failed!\n");
//     }
}


void display_done_ISR(uint8_t cmd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR(xDisplayISRTask, &xHigherPriorityTaskWoken);
printf("ISR!\n");
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
    display.State = DISPLAY_STATE_TURNING_ON;
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
    display.State = DISPLAY_STATE_IDLE;
}

void backlight_set(uint16_t brightness)
{
    display.Brightness = brightness;
    
    // set the display pwm value
    hw_backlight_set(brightness);
}

void build_scanline(void);

uint16_t display_checkerboard(char *frameData, uint8_t invert)
{
    int gridx = 0;
    int gridy = 0;
    int count = 0;

    uint8_t forCol = RED;
    uint8_t backCol = GREEN;
    
//     if (invert)
//     {
        for(uint16_t i = 0; i < 24192; i++)
        {
            frameData[i] = rebbleOS[i];
        }
        build_scanline();
        return;
//     }
    
    
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


void vDisplayISRProcessor(void *pvParameters)
{
    uint32_t ulNotificationValue;
    
    while(1)
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(30));

        if (ulNotificationValue == 1)
        {
            printf("asasdasdasdasd state %d\n", display.State);
            switch(display.State)
            {
                case DISPLAY_STATE_FRAME_INIT:
                    display_send_frame();
                    // DO NOT yield to the task scheduler, go back around the loop and 
                    // wait until the frame is away
                    continue;
            }
//            printf("ISR Handler Idle\n");
            //display.State = DISPLAY_STATE_IDLE;
//            xTaskNotifyGive(xDisplayCommandTask);
        }
    }
}


void test(char *frameData, uint16_t pos, uint32_t col)
{
//     for(uint16_t i = 0; i < 24192; i++)
//     {
//         frameData[i] = rebbleOS[i];
//     }
    frameData[pos] = col;
    return;
}

// state machine thread for the display
// keeps track of init, flash and frame management
void vDisplayCommandTask(void *pvParameters)
{
    uint8_t data;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    display.State = DISPLAY_STATE_IDLE;
    uint8_t invert = 1;
    //int len = 0;
    uint32_t ulNotificationValue;
        //int test = 0;
    while(1)
    {
//         printf("astate %d\n", display.State);
//         // add a semaphore here to block the irq acks
//         if (display.State != DISPLAY_STATE_IDLE)
//         {
//             ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(30));
//             if(ulNotificationValue == 0)
//             {
//                 printf("ERROR: No ISR Responses in 30ms\n");
//                 display.State = DISPLAY_STATE_IDLE;
//                 continue;
//             }
//             else
//             {
//                 printf(":) GOT ISR\n");
//             }
//             
//             display.State = DISPLAY_STATE_IDLE;
//         }
        // commands to be exectuted are send to this queue and processed
        // one at a time
        if (xQueueReceive(xQueue, &data, xMaxBlockTime))
        {
            printf("CMD\n");
            //display.State = data;
            switch(data)
            {
                case DISPLAY_CMD_DRAW:
                    //display_checkerboard(display.DisplayBuffer, 1);
                    //display_start_frame();
                    init_gui();
                    break;
            }
        }
        else
        {
            // nothing emerged from the buffer
            printf("Display heartbeat\n");
            // do one second(ish) maint tasks
/*            invert = !invert;
            //vibrate_enable(invert);
            display_checkerboard(display.DisplayBuffer, invert);

            display_start_frame();
            */
        }        
    }
}

void build_scanline(void)
{
    char colBuffer[169];
    
    for (int x = 0; x < display.NumCols; x++)
    {
        // backup the column
        for (int i = 0; i < 168; i++)
        {
            colBuffer[i] = display.DisplayBuffer[(x * display.NumRows) + i];
        }
        
        for (int y = 0; y < display.NumRows; y += 2)
        {
            uint16_t pos_half_lsb = (x * display.NumRows) + (y / 2);
            uint16_t pos_half_msb = (x * display.NumRows) + (display.NumRows / 2) + (y / 2);
    
        
            // for every column
            // get the first byte of each msb and msb
            uint8_t lsb0 = (colBuffer[y] & (0b00101010)) >> 1;
            uint8_t msb0 = (colBuffer[y] & (0b00010101));
            
            uint8_t lsb1 = (colBuffer[y + 1] & (0b00101010)) >> 1;
            uint8_t msb1 = (colBuffer[y + 1] & (0b00010101));
            
            display.DisplayBuffer[pos_half_lsb] = lsb0 << 1 | lsb1;
            display.DisplayBuffer[pos_half_msb] = msb0 << 1 | msb1;
            
//             uint8_t lsb0 = (colBuffer[y] & (0b0010101));
//             uint8_t msb0 = (colBuffer[y] & (0b000101010)) >> 1;
//             
//             uint8_t lsb1 = (colBuffer[y + 1] & (0b0010101));
//             uint8_t msb1 = (colBuffer[y + 1] & (0b000101010)) >> 1;
//             
//             display.DisplayBuffer[pos_half_lsb] = lsb1 << 1 | lsb0;
//             display.DisplayBuffer[pos_half_msb] = msb1 << 1 | msb0;
        }
    }
}


// GUI related tests

UG_GUI gui;

void UserSetPixel (UG_S16 x, UG_S16 y, UG_COLOR c)
{
    y = 167 - y; // invert the y as it renders upside down
    uint16_t pos = (x * display.NumRows) + y;
    uint16_t pos_half_lsb = (x * display.NumRows) + (y / 2);
    uint16_t pos_half_msb = (x * display.NumRows) + (display.NumRows / 2) + (y / 2);
    
    // take the rgb888 and turn it into something the display can use
    uint16_t red = c >> 16;
    uint16_t green = (c & 0xFF00) >> 8;
    uint16_t blue = (c & 0x00FF);
    
    // scale from rgb to pbl
    red = red / (255 / 3);
    green = green / (255 / 3);
    blue = blue / (255 / 3);

    uint8_t odd = !(y % 2);
    uint16_t fullbyte = 0;
    
    fullbyte = (red << 4 | green << 2 | blue << 0);
    
    uint8_t lsb = (fullbyte & (0b00010101));
    uint8_t msb = (fullbyte & (0b00101010)) >> 1;
    
    uint8_t olsb = display.DisplayBuffer[pos_half_lsb] & (0b000101010 >> odd);
    uint8_t omsb = display.DisplayBuffer[pos_half_msb] & (0b000101010 >> odd);
    display.DisplayBuffer[pos_half_lsb] = olsb | lsb << odd;
    display.DisplayBuffer[pos_half_msb] = omsb | msb << odd;
}

int init_gui(void)
{
  /* Configure uGUI */
  UG_Init(&gui, UserSetPixel , display.NumCols, display.NumRows);

  /* Draw text with uGUI */
  UG_FontSelect(&FONT_8X14);
  UG_ConsoleSetArea(0, 0, display.NumCols-1, display.NumRows-1);
  UG_ConsoleSetBackcolor(C_BLACK);
  UG_ConsoleSetForecolor(C_GREEN);
  UG_ConsolePutString("RebbleOS...\n");
  UG_ConsoleSetForecolor(C_GREEN);
  UG_ConsolePutString("Test 1\n");
  UG_ConsoleSetForecolor(C_BLUE);
  UG_ConsolePutString(":D\n");
  UG_ConsoleSetForecolor(C_RED);
  UG_ConsolePutString("Time: 00:31:00\n");
  
display_start_frame();
  return 0;
}
