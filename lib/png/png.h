#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gbitmap/gbitmap.h"

void png_to_gbitmap(n_GBitmap *bitmap, uint8_t *raw_buffer, size_t png_size);

