#ifndef _PORTMACRO_H
#define _PORTMACRO_H

#include "portmacro_cmsis.h"

inline __attribute__(( always_inline)) static BaseType_t xPortIsInsideInterrupt( void )
{
    uint32_t ulCurrentInterrupt;
    BaseType_t xReturn;

	/* Obtain the number of the currently executing interrupt. */
	__asm volatile( "mrs %0, ipsr" : "=r"( ulCurrentInterrupt ) );

	if( ulCurrentInterrupt == 0 )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	return xReturn;
}

#endif
