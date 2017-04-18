#pragma once
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#define graphics_context_set_fill_color n_graphics_context_set_fill_color
#define graphics_fill_rect n_graphics_fill_rect
#define graphics_context_set_stroke_color n_graphics_context_set_stroke_color
#define graphics_context_set_stroke_width n_graphics_context_set_stroke_width
#define graphics_context_set_antialiased n_graphics_context_set_antialiased

#define graphics_fill_circle n_graphics_fill_circle
#define graphics_draw_circle n_graphics_draw_circle
#define graphics_draw_line n_graphics_draw_line

#define GColorFromRGBA n_GColorFromRGBA
#define GColorFromRGB n_GColorFromRGB
#define GColor8 n_GColor8


// text redefines
#define GTextOverflowMode n_GTextOverflowMode
#define GFont n_GFont
#define GTextAlignment n_GTextAlignment
#define GTextAttributes n_GTextAttributes
#define graphics_draw_text n_graphics_draw_text

// math
#define TRIG_MAX_RATIO 0xffff
#define TRIG_MAX_ANGLE 0x10000

#define GContext n_GContext


#define GSize n_GSize
#define GColor n_GColor
#define GRect n_GRect
#define GPoint n_GPoint

#define GColorWhite n_GColorWhite
#define GColorRed   n_GColorRed
#define GColorNone  n_GColorNone
#define GColorBlack n_GColorBlack
#define GColorBlue  n_GColorBlue

#define GCornerNone n_GCornerNone

#define graphics_context_set_text_color n_graphics_context_set_text_color
#include "gbitmap.h"
