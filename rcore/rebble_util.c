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
