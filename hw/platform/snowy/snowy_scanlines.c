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

extern display_t display;

void scanline_convert_buffer(uint8_t xoffset, uint8_t yoffset)
{
    int i = 0;
    // go through each x and remap it to the new y
    
    // from    (Backbuffer)
    // x0: [y0,1,2,3,4..]. x1: [y1,2,3,4..]..
    // to    (nativebuffer)
    // y0: [x0,1,2,3,4..]. y1: [x1,2,3,4..]..    
    for (uint16_t yi = 0; yi < display.NumRows; yi++)
    {
        uint16_t y = 167 - yi;
        for (int x = 0; x < display.NumCols; x++)    
        {
            // non destructively convert a single pixel in the buffer
            uint16_t pos_half_lsb = (x * display.NumRows) + (y / 2);
            uint16_t pos_half_msb = (x * display.NumRows) + (display.NumRows / 2) + (y / 2);
                    
            // below sets to native display format
            
            uint8_t odd = !(y % 2);
            uint8_t fullbyte = display.BackBuffer[i];
                        
            uint8_t lsb = (fullbyte & (0b00010101));
            uint8_t msb = (fullbyte & (0b00101010)) >> 1;
            
            // or the new pixel into the existing framebuffer value so we don't smash it
            uint8_t olsb = display.DisplayBuffer[pos_half_lsb] & (0b000101010 >> odd);
            uint8_t omsb = display.DisplayBuffer[pos_half_msb] & (0b000101010 >> odd);
            
            // there are two bytes in each msb/lsb pair. We shift the incoming value
            // by "odd" bytes. Odd is 0 or 1 based on the y value being an odd/even
            // odd values  (1,3,5...) occupy the left-most bits of the byte: & 101010
            // even values (0,2,4...) occupy the right-most bits of the byte: & 010101
            display.DisplayBuffer[pos_half_lsb] = olsb | lsb << odd;
            display.DisplayBuffer[pos_half_msb] = omsb | msb << odd;
            
            i++;
        }
    }
}



// uGUI gives us a single pixel and color c in RGB888
// the cordinates of the pixel are used to calculate the position of 
// the real framebuffer position.
// Because of the way the scanlines are encoded [LSB0|LSB1...  MSB0|MSB1...]
// we will convert the pixel into its lsb and msb, and or it into the existing buffer
void scanline_rgb888pixel_to_frambuffer(UG_S16 x, UG_S16 y, UG_COLOR c)
{
    if (x > display.NumCols || y > display.NumRows)
        return;
    
    // write to the pixel buffer in y0 [x0, 1, 3], y1 [x1, 1 2 3]
    uint16_t pos = (y * display.NumCols) + x;
        
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
