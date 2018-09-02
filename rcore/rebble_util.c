#include <stdlib.h>
#include "rebbleos.h"
#include "librebble.h"

int32_t read_32(uint8_t *addr)
{
    int32_t val = ((int32_t)addr[0] ) | 
                  ((int32_t)addr[1] << 8) | 
                  ((int32_t)addr[2] << 16 ) | 
                  ((int32_t)addr[3] << 24);
    return val;
}

void write_32(uint8_t *addr, int32_t val)
{
    addr[0] = (val)        & 0xFF;
    addr[1] = (val >> 8)   & 0xFF;
    addr[2] = (val >> 16)  & 0xFF;
    addr[3] = (val >> 24)  & 0xFF;
}


/* Do delay for nTime milliseconds 
 * NOT safe unless scheduler is running
 */
void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
    return;   
}

uint32_t map_range(uint32_t input, uint32_t input_start, uint32_t input_end, uint32_t output_start, uint32_t output_end)
{
    int32_t input_range = input_end - input_start;
    int32_t output_range = output_end - output_start;

    int32_t output = (((input - input_start) * output_range) / (input_range + output_start));
    if (output < 0)
        output = output_start;
    if (output > output_end)
        output = output_end;
    return output;
}
