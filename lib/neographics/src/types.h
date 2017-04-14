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
#include "types/color.h"
#include "types/cornermask.h"
#include "types/point.h"
#include "types/rect.h"
#include "types/size.h"

/*-----------------------------------------------------------------------------.
|                                                                              |
|                                   Types                                      |
|                                                                              |
|    This file should contain all `typedef`s relevant to graphics in           |
|    general *except* DrawCommands. It should also contain all macros          |
|    relevant to creating or modifying those types.                            |
|                                                                              |
`-----------------------------------------------------------------------------*/
