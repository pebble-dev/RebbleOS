/* log.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"


void app_log_trace(uint8_t level, const char *filename, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf("%x\n",fmt);
    // TODO get defines
    if (level == APP_LOG_LEVEL_DEBUG)
        printf("DEBUG ");
    else 
        printf("INFO "); 
    printf(filename, ar);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}

void app_log(uint8_t lvl, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}
