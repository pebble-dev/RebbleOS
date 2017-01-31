#ifndef __VIBRATE_H
#define __VIBRATE_H
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
#define VIBRATE_CMD_PATTERN_1 1
#define VIBRATE_CMD_PATTERN_2 2
#define VIBRATE_CMD_STOP      3

void vibrate_pattern(uint16_t *pattern, uint8_t patternLength);
void vibrate_enable(uint8_t enabled);
void vibrate_init(void);

// Main task thread
void vVibratePatternTask(void *pvParameters);

static const uint16_t VIBRO_SHORT[] = { 255, 1000 };
static const uint16_t VIBRO_TAP_TAP[] = { 255, 750, 0, 750, 255, 750 };

#endif
