#include "display.h"
#include "librebble.h"
#include "uuid.h"

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


void uuid_to_string(const Uuid *uuid, char *buffer)
{
    uint8_t *id = (uint8_t *)uuid;
    if (!buffer)
        return;

    if (uuid_null((Uuid *)id))
    {
        snprintf(buffer, 13, "{NULL UUID}");
        return;
    }

    snprintf(buffer, UUID_STRING_BUFFER_LENGTH, 
             "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                 uuid->byte0, uuid->byte1, uuid->byte2, uuid->byte3,
                 uuid->byte4, uuid->byte5, uuid->byte6, uuid->byte7,
                 uuid->byte8, uuid->byte9, uuid->byte10, uuid->byte11,
                 uuid->byte12, uuid->byte13, uuid->byte14, uuid->byte15);
}

bool uuid_is_int(const Uuid *uuid, const uint8_t c)
{
    uint8_t i;
    for(i = 0; i < UUID_SIZE; i++)
    {
        if (((uint8_t *)uuid)[i] != c)
            break;
    }
    return i + 1 == UUID_SIZE;
}

bool uuid_equal(const Uuid *uu1, const Uuid *uu2)
{
    for (uint8_t i = 0; i < UUID_SIZE; i++) {
        if (((uint8_t *)uu1)[i] != ((uint8_t *)uu2)[i])
            return false;
    }
    return true;
}

bool uuid_null(const Uuid *uuid)
{
    return uuid_is_int(uuid, 0);
}

/* String utils */

uint8_t pascal_string_to_string(uint8_t *result_buf, uint8_t *source_buf)
{
    uint8_t len = (uint8_t)source_buf[0];
    /* Byte by byte copy the src to the dest */
    for(int i = 0; i < len; i++)
        result_buf[i] = source_buf[i+1];
    
    /* and null term it */
    result_buf[len] = 0;
    
    return len + 1;
}

uint8_t pascal_strlen(char *str)
{
    return (uint8_t)str[0];
}
