/* snowy_scanlines.c
 * Scanline conversion helper routines for Pebble Time (snowy) display
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "display.h"
#include "snowy_display.h"

extern display_t display;

/*
 * Bulk convert the buffer from its native format for a sigle column
 * (y0: xxxxxxx
 *  y1: xxxxxxx)
 * to
 * (y0: xxxxxxx
 *  y1: xxxxxxx)
 * In LSB / MSB format
 */
void _scanline_convert_row(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t row_index)
{
    uint8_t r0_fullbyte, r1_fullbyte, lsb, msb;
    uint32_t row_offset = row_index * DISPLAY_COLS;

    // For each column in the row, grab two consecutive bytes
    // Each pair of bytes is then or'd to form a pair of formatted values
    // suitable for the FPGA.
    // They are then pushed into the row buffer. 
    // LSB block filling the first half, MSB the second half
    // [LSB0 LSB1..... | half | MSB0 MSB1.....]
    for (uint16_t xi = 0; xi < DISPLAY_COLS; xi+=2)
    {
        r1_fullbyte = frame_buffer[row_offset + xi];
        r0_fullbyte = frame_buffer[row_offset + xi + 1];
        
        lsb = (r0_fullbyte & (0b00101010)) >> 1 | (r1_fullbyte & (0b00101010));
        msb = (r0_fullbyte & (0b00010101)) | (r1_fullbyte & (0b00010101)) << 1;
            
        out_buffer[xi/2] = lsb;
        out_buffer[(xi/2) + DISPLAY_COLS / 2] = msb;
    }
}

void _scanline_convert_column(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t column_index)
{
    int i = 0;
    uint16_t pos_half_lsb = 0;
    uint16_t pos_half_msb = 0;
    
    uint16_t y;
    uint8_t r0_fullbyte, r1_fullbyte, lsb, msb;
    uint16_t halfrows = DISPLAY_ROWS / 2;
    
    // from    (Backbuffer)
    // y0: [x0,x1,x2,x3,x4..]. y1: [x1,x2,x3,x4..]..
    // to    (nativebuffer, stored in columns order)
    // x0: [y0,y1,y2,y3,y4..]. x1: [y1,y2,y3,y4..]..    
    for (uint16_t yi = 0; yi < DISPLAY_ROWS; yi+=2)
    {
        y = DISPLAY_ROWS - 1 - yi;
        uint16_t halfy = y / 2;

        // we store the actual buffer in columns order
        // where the columns buffer is split with lsb/msb encoding
        pos_half_lsb = halfy;
        pos_half_msb = halfrows + halfy;
        
        r0_fullbyte = frame_buffer[column_index + i];
        r1_fullbyte = frame_buffer[column_index + i + DISPLAY_COLS];
        
        lsb = (r0_fullbyte & (0b00101010)) >> 1 | (r1_fullbyte & (0b00101010));
        msb = (r0_fullbyte & (0b00010101)) | (r1_fullbyte & (0b00010101)) << 1;
        
        out_buffer[pos_half_lsb] = lsb;
        out_buffer[pos_half_msb] = msb;

        // skip the next y column as we processed it already
        i += 2 * DISPLAY_COLS;
    }
}

void scanline_convert(uint8_t *out_buffer, uint8_t *frame_buffer, uint8_t index)
{
#if defined(REBBLE_PLATFORM_CHALK)
    _scanline_convert_row(out_buffer, frame_buffer, index);
#elif defined(REBBLE_PLATFORM_SNOWY)
    _scanline_convert_column(out_buffer, frame_buffer, index);
#else
    assert(!"I don't know how to drive this platform!");
#endif
}
