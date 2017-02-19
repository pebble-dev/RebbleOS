#ifndef __GRAPHICS_BITMAP_H__
#define __GRAPHICS_BITMAP_H__

#include "types.h"

struct n_GRect;

typedef struct GBitmap
{
    void *addr;
    uint16_t row_size_bytes;
    uint16_t info_flags;
    n_GRect bounds;
} GBitmap;


#define GBitmapFormat uint8_t



#endif
