#ifndef _PLATFORM_FREERTOS_H
#define _PLATFORM_FREERTOS_H

#include "nrf_soc.h"
#include "app_util_platform.h"

extern uint32_t SystemCoreClock;

#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY _PRIO_APP_HIGH

#endif
