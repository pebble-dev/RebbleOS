/* snowy_scanlines.c
 * Scanline conversion helper routines for Pebble Time (snowy) display
 * RebbleOS
 *
 * Authors: Barry Carter <barry.carter@gmail.com>
 *          Carson Katri <me@carsonkatri.com>
 */

#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "chalk_display.h"


extern display_t display;

/*
 * Bulk convert the buffer from its native format for a single column
 * (y0: xxxxxxx
 *  y1: xxxxxxx)
 * to
 * (x0: yyyyyyy
 *  x1: yyyyyyy)
 */
void scanline_convert_column(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t column_index)
{
    uint16_t pos_half_lsb = 0;
    uint16_t pos_half_msb = 0;
    
    uint16_t y;
    uint8_t r0_fullbyte, r1_fullbyte, lsb, msb;
    uint16_t halfrows = DISPLAY_ROWS / 2;
    int i = 0;

    // from    (Backbuffer)
    // y0: [x0,x1,x2,x3,x4..]. y1: [x1,x2,x3,x4..]..
    // to    (nativebuffer, stored in columns order)
    // x0: [y0,y1,y2,y3,y4..]. x1: [y1,y2,y3,y4..]..
    for (uint16_t yi = 0; yi < DISPLAY_ROWS; yi+=2)
    {
        y = DISPLAY_ROWS - yi;
        uint16_t halfy = y / 2;
        
        // we store the actual buffer in columns order
        // where the columns buffer is split with lsb/msb encoding
        pos_half_lsb = halfy;
        pos_half_msb = halfy + halfrows;
        
        r0_fullbyte = frame_buffer[column_index + i];
        r1_fullbyte = frame_buffer[column_index + i];
        
        lsb = (r0_fullbyte & (0b00010101)) | (r1_fullbyte & (0b00010101)) << 1;
        msb = (r0_fullbyte & (0b00101010)) >> 1 | (r1_fullbyte & (0b00101010));
        
        out_buffer[pos_half_lsb] = lsb;
        out_buffer[pos_half_msb] = msb;
        
        // skip the next y column as we processed it already
        i += 288;
    }
}
