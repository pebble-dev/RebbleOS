#pragma once
#include "stm32f4xx.h"

#define RTOS_HEAP_SIZE 30 * 1024

/* Size of the app + stack + heap of the running app. 
   IN BYTES
 */ 
#define MEMORY_SIZE_APP           90000
#define MEMORY_SIZE_WORKER        10500
#define MEMORY_SIZE_OVERLAY       18500

/* Size of the stack in WORDS */
#define MEMORY_SIZE_APP_STACK     3000
#define MEMORY_SIZE_WORKER_STACK  250
#define MEMORY_SIZE_OVERLAY_STACK 450


#define MEMORY_SIZE_APP_HEAP      MEMORY_SIZE_APP - (MEMORY_SIZE_APP_STACK * 4)
#define MEMORY_SIZE_WORKER_HEAP   MEMORY_SIZE_WORKER - (MEMORY_SIZE_WORKER_STACK * 4)
#define MEMORY_SIZE_OVERLAY_HEAP  MEMORY_SIZE_OVERLAY - (MEMORY_SIZE_OVERLAY_STACK * 4)

// flash regions
#define REGION_PRF_START        0x200000
#define REGION_PRF_SIZE         0x1000000
// DO NOT WRITE TO THIS REGION
#define REGION_MFG_START        0xE0000
#define REGION_MFG_SIZE         0x20000
// Resource start
#define REGION_RES_START        0x380000
#define REGION_RES_SIZE         0x7D000 // TODO


#define REGION_FS_START         0x400000
#define REGION_FS_PAGE_SIZE     0x2000
#define REGION_FS_N_PAGES       ((0x1000000 - REGION_FS_START) / REGION_FS_PAGE_SIZE)
#define REGION_FS_ERASE_SIZE    (32*1024)

#define REGION_APP_RES_START    0xB3A000
#define REGION_APP_RES_SIZE     0x7D000


/* The size of the page that holds an apps header table. This is the amount before actual app content e.g
 0x0000  Resource table header
 0x1000  Resource data start
 */
#define APP_RES_START           0x1000

/* System resource table offset */
#define RES_START               0x200C
#define SPLASH_RESOURCE_ID      474
/* When we load a font from flash, is it offset from the given offset in the header? Seems so */
// Some fonts? confused
//#define APP_FONT_START          0x1C
// #define APP_FONT_START          0x00

#define Bank1_NOR_ADDR ((uint32_t)0x60000000)

/* Use the prefix CCRAM to force the memory of an object to be pushed
 * into memory bank 2. Note bank 2 is NOT DMA capable
 */
#define MEM_REGION_CCRAM __attribute__((section(".ccmram")))


//Snowy uses OC1 for backlight
#define BL_TIM_CH 1

static inline uint8_t is_interrupt_set(void)
{
    return ((volatile int)(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)) != 0 ;
}
