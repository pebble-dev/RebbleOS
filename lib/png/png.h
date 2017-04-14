#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pebble.h>


void png_to_gbitmap(GBitmap *bitmap, uint8_t *raw_buffer, size_t png_size);

