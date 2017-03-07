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
#include "display.h"
#include "snowy_display.h"
#include "ugui.h"

extern display_t display;

/*
 * Bulk convert the buffer from its native format
 * (y0: xxxxxxx
 *  y1: xxxxxxx)
 * to
 * (x0: yyyyyyy
 *  x1: yyyyyyy)
 */
void scanline_convert_buffer(uint8_t xoffset, uint8_t yoffset)
{
    int i = 0;
    uint16_t pos_half_lsb = 0;
    uint16_t pos_half_msb = 0;
    
    uint16_t y;
    uint8_t r0_fullbyte, r1_fullbyte, lsb, msb;
    uint16_t halfrows = DISPLAY_ROWS / 2;
    // go through each x and remap it to the new y
    
    // from    (Backbuffer)
    // y0: [x0,x1,x2,x3,x4..]. y1: [x1,x2,x3,x4..]..
    // to    (nativebuffer, stored in columns order)
    // x0: [y0,y1,y2,y3,y4..]. x1: [y1,y2,y3,y4..]..    
    for (uint16_t yi = 0; yi < DISPLAY_ROWS; yi+=2)
    {
        y = DISPLAY_ROWS - 1 - yi;
        uint16_t px = 0;
        uint16_t halfy = y / 2;
        for (int x = 0; x < DISPLAY_COLS; x++)
        {
            // we store the actual buffer in columns order
            // where the columns buffer is split with lsb/msb encoding
            pos_half_lsb = px + halfy;
            pos_half_msb = px + halfrows + halfy;
            
            r0_fullbyte = display.BackBuffer[i];
            r1_fullbyte = display.BackBuffer[i + DISPLAY_COLS];
            
            lsb = (r0_fullbyte & (0b00010101)) | (r1_fullbyte & (0b00010101)) << 1;
            msb = (r0_fullbyte & (0b00101010)) >> 1 | (r1_fullbyte & (0b00101010));
            
            display.DisplayBuffer[pos_half_lsb] = lsb;
            display.DisplayBuffer[pos_half_msb] = msb;
            
            px += DISPLAY_ROWS;
            i++;
        }
        // skip the next y column as we processed it already
        i += DISPLAY_COLS;
    }
}



// uGUI gives us a single pixel and color c in RGB888
// the cordinates of the pixel are used to calculate the position of 
// the real framebuffer position.
// Because of the way the scanlines are encoded [LSB0|LSB1...  MSB0|MSB1...]
// we will convert the pixel into its lsb and msb, and or it into the existing buffer
void scanline_rgb888pixel_to_frambuffer(UG_S16 x, UG_S16 y, UG_COLOR c)
{
    if (x > DISPLAY_COLS || y > DISPLAY_ROWS)
        return;
    
    // write to the pixel buffer in y0 [x0, 1, 3], y1 [x1, 1 2 3]
    uint16_t pos = (y * DISPLAY_COLS) + x;
        
    // take the rgb888 and turn it into something the display can use
    uint16_t red = c >> 16;
    uint16_t green = (c & 0xFF00) >> 8;
    uint16_t blue = (c & 0x00FF);
    
    // scale from rgb to pbl
    red = red / (255 / 3);
    green = green / (255 / 3);
    blue = blue / (255 / 3);

    // for now write to the pixel buffer in raw format
    display.BackBuffer[pos] = red << 4 | green << 2 | blue;

    return;
}
