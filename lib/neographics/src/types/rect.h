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
#include "point.h"
#include "size.h"

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                    Rect                                      |
|                                                                              |
`-----------------------------------------------------------------------------*/

typedef struct n_GRect {
    n_GPoint origin;
    n_GSize size;
} n_GRect;

#define n_GRect(x, y, w, h) ((n_GRect) {{x, y}, {w, h}})

#define n_grect_standardize(a) \
    ((a).size.w >= 0 \
        ? (a).size.h >= 0 \
            /* normal */ \
            ? (n_GRect) { {  (a).origin.x,      (a).origin.y }, \
                          {  (a).size.w,        (a).size.h     }} \
            /* switch vertically */ \
            : (n_GRect) { {  (a).origin.x,      (a).origin.y + (a).size.h - 1 }, \
                          {  (a).size.w,       -(a).size.h + 2 }} \
        : (a).size.h >= 0 \
            /* switch horizontally */ \
            ? (n_GRect) { {  (a).origin.x + (a).size.w - 1, (a).origin.y }, \
                          { -(a).size.w + 2,    (a).size.h     }} \
            /* switch horizontally and vertically */ \
            : (n_GRect) { {  (a).origin.x + (a).size.w - 1, (a).origin.y + (a).size.h - 1 }, \
                          { -(a).size.w + 2,   -(a).size.h + 2 }})
