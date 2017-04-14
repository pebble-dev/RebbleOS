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
#include <pebble.h>

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                 Neographics                                  |
|                                                                              |
`-----------------------------------------------------------------------------*/

#include "common.h"
#include "context.h"

#include "types.h"

#include "primitives/line.h"
#include "primitives/circle.h"
#include "primitives/rect.h"

#include "path/path.h"

#include "draw_command/draw_command.h"

#include "fonts/fonts.h"
#include "text/text.h"
