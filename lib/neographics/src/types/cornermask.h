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
|                                 CornerMask                                   |
|                                                                              |
`-----------------------------------------------------------------------------*/

typedef enum n_GCornerMask {
    n_GCornersNone       = 0, // support all grammatical preferences
    n_GCornerNone        = 0,
    n_GCornerTopLeft     = 0b0001,
    n_GCornerTopRight    = 0b0010,
    n_GCornerBottomLeft  = 0b0100,
    n_GCornerBottomRight = 0b1000,
    n_GCornersTop        = 0b0011,
    n_GCornersBottom     = 0b1100,
    n_GCornersLeft       = 0b0101,
    n_GCornersRight      = 0b1010,
    n_GCornersAll        = 0b1111,
} n_GCornerMask;
