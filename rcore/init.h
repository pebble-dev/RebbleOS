#pragma once
/* init.h
 * routines for [...]
 * RebbleOS
 */
//Let's make sure the user is using an 'up to date' version of the compiler


#define VERSION "v0.0.0.2"

#if __GNUC__ < 8
    #error It appears you are using a version of arm-none-eabi-gcc that is incompatible with with the RebbleOS project. \
    Please find an updated version by using your respective package manager or going to the developer.arm.com download page.
#endif



// Reset the watchdog timer manually
void watchdog_reset(void);


void rebbleos_init(void);
void os_module_init_complete(uint8_t result);

#define INIT_RESP_OK            0
#define INIT_RESP_ASYNC_WAIT    1
#define INIT_RESP_NOT_SUPPORTED 2
#define INIT_RESP_ERROR         3 
