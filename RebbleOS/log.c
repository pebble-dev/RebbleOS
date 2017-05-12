/* log.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"


/*
 * Print log output (like APP_LOG:   INFO filename.c message)
 */
void app_log_trace(uint8_t level, const char *filename, uint32_t line_no, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    
    // TODO get defines
    if (level == APP_LOG_LEVEL_DEBUG)
        printf("DEBUG ");
    else 
        printf("INFO "); 
    
    printf("%s|ln %d|", filename, line_no);
    
    char buf[80];
    sprintf(buf, fmt, ar);
    printf(buf);
    printf("\n");
//     vprintf(fmt, ar); // TODO we don't have this
    va_end(ar);
}

/*
 * Printf with a log level int
 */
void app_log(uint8_t lvl, const char *fmt, ...)
{
    va_list ar;
    va_start(ar, fmt);
    printf(fmt, ar);
    printf("\n");
    va_end(ar);
}
