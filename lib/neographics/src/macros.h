/*\
|*|
|*|   Neographics: a tiny graphics library.
|*|   Copyright (C) 2016 Johannes Neubrand <johannes_n@icloud.com>
|*|
|*|   This program is free software; you can redistribute it and/or
|*|   modify it under the terms of the GNU General Public License
|*|   as published by the Free Software Foundation; either version 2
|*|   of the License, or (at your option) any later version.
|*|
|*|   This program is distributed in the hope that it will be useful,
|*|   but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|   GNU General Public License for more details.
|*|
|*|   You should have received a copy of the GNU General Public License
|*|   along with this program; if not, write to the Free Software
|*|   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
|*|
\*/

#pragma once

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                    Macros                                    |
|                                                                              |
|    General-purpose macros and system-specific things.                        |
|                                                                              |
`-----------------------------------------------------------------------------*/

#define __BOUND_NUM(a, b, c) ((b) <= (a) ? (a) : ((b) >= (c) ? (c) : (b)))
#ifdef PBL_BW
#define __ARGB_TO_INTERNAL(a) ((((a) & 0b111111) == 0b111111) ? 0b11111111 :\
                               (((a) & 0b111111) == 0) ? 0b00000000 : 0b01010101)
#endif

#ifdef PBL_RECT
    #ifndef __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT
        #ifdef PBL_BW
            #define __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT (160 / 8)
        #else
            #define __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT (144)
        #endif
    #endif
    #define __SCREEN_WIDTH 144
    #define __SCREEN_HEIGHT 168
#else
    #define __SCREEN_FRAMEBUFFER_ROW_BYTE_AMOUNT 180
    #define __SCREEN_WIDTH 180
    #define __SCREEN_HEIGHT 180
#endif
