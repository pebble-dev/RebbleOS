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

#include "rebbleos.h"

extern display_t display;
extern SystemStatus system_status;
extern SystemSettings system_settings;

static UG_GUI gui;


/*
 * Initialise the uGUI component and start the draw
 */
void gui_init(void)
{   
    /* Configure uGUI */
    UG_Init(&gui, scanline_rgb888pixel_to_frambuffer, display.NumCols, display.NumRows);

    /* Draw text with uGUI */
    UG_FontSelect(&FONT_8X14);

    neographics_init(display.BackBuffer);
}

