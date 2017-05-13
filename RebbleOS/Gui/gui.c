/* gui.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"

extern SystemStatus system_status;
extern SystemSettings system_settings;


/*
 * Initialise the uGUI component and start the draw
 */
void gui_init(void)
{   
    neographics_init(display_get_buffer());
}

