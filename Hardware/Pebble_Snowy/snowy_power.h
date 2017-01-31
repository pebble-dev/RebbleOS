#ifndef __SNOWY_POWER_H
#define __SNOWY_POWER_H
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

#define REG_BUCK2_CFG  0x0F
#define REG_BUCK2_VSET 0x10

typedef struct {
    
    // I2C Stuff
    uint8_t Address;   
    GPIO_TypeDef *PortI2C;
    uint16_t PinSCL;
    uint16_t PinSDA;

    uint16_t PinIntn;   // power interrupt
    uint16_t PinReset;  // reset the max14690 (low)
    uint16_t PinMPC0;   // external peripheral control 0
    uint16_t PinMPC1;   // external peripheral control 1
    uint16_t PinPFN1;   // Fn1
    uint16_t PinPFN2;   // Fn2
    uint16_t PinMON;    // Monitor
} max14690_t;

void hw_power_init(void);
void max14690_init(void);


#endif
