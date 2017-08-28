#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "tintin.h"
#include "stm32_buttons.h"
#include "driver.h"

#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

#define REGION_PRF_START    0x200000
#define REGION_PRF_SIZE     0x1000000
// DO NOT WRITE TO THIS REGION
#define REGION_MFG_START    0xE0000
#define REGION_MFG_SIZE     0x20000
// Resource start
#define REGION_RES_START    0x280000
#define REGION_RES_SIZE     0x7D000

#define REGION_FS_START         0x2c0000
#define REGION_FS_PAGE_SIZE     0x1000
#define REGION_FS_N_PAGES       ((0x3E0000 - REGION_FS_START) / REGION_FS_PAGE_SIZE)

#define REGION_APP_RES_START    0xB3A000
#define REGION_APP_RES_SIZE     0x7D000

// XXX TODO these are from Snowy. NOT correct
/* App slots are a chunk of flash that holds the information.
    Seems to be paged 
*/
#define APP_SLOT_0_START        0xB3E000
#define APP_SLOT_8_START        0xc34000
#define APP_SLOT_16_START       0xc82000
#define APP_SLOT_24_START       0xe7e000


/* Each app section (resouce, binary) has this header junk */
#define APP_HEADER_BIN_OFFSET   0x59

/* The size of the slot on flash for an app. */
#define APP_SLOT_SIZE           0x48000

/* The size of the page that holds an apps header table. This is the amount before actual app content e.g
 0x0000  Resource table header
 0x1000  Resource data start
 */
#define APP_RES_START           0x1000

/* This is the NEGATIVE (yes, sigh) offset of the resource table from the app binary */
#define APP_RES_TABLE_OFFSET    0x6000

/* When we load a font from flash, is it offset from the given offset in the header? Seems so */
#define APP_FONT_START          0x1C


// BACK TO CORRECT?

#define RES_START           0x100C

/* Size of the app + stack + heap of the running app. 
   IN BYTES
 */ 
#define MAX_APP_MEMORY_SIZE     40000

/* Size of the stack in WORDS */
#define MAX_APP_STACK_SIZE      2000



#endif
