#pragma once
/* test.h
 *
 * RebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>.
 */

#include "librebble.h"
#include "display.h"
#include "backlight.h"
#include "action_bar_layer.h"

#define BTN_SELECT_PRESS 1
#define BTN_BACK_PRESS   2
#define BTN_UP_PRESS     3
#define BTN_DOWN_PRESS   4

#define APP "RebbleOS"
#define TITLE "Notification:"
#define BODY "This is an example alert. You can use the buttons to scroll through it."
#define COLOR GColorFromRGB(0, 170, 0)
#define ICON_ID 19

void notif_main(void);

// , , ,
