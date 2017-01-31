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
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "snowy_power.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_tim.h>

// useful?
// https://developer.mbed.org/users/switches/code/MAX14690/file/666b6c505289/MAX14690.h

// massive work in progress

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
void I2C_stop(I2C_TypeDef* I2Cx);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);

max14690_t max1690 = {
    .Address    = 0x50,
    .PinIntn    = GPIO_Pin_1,
    .PinReset   = GPIO_Pin_1, // unknown
    .PinSCL     = GPIO_Pin_6,
    .PinSDA     = GPIO_Pin_9,
    // unknown
    .PinMPC0    = GPIO_Pin_8,
    .PinMPC1    = GPIO_Pin_8,
    .PinPFN1    = GPIO_Pin_8,
    .PinPFN2    = GPIO_Pin_8,
    .PinMON     = GPIO_Pin_8,
};

void hw_power_init(void)
{   
    power_i2c_init();
    max14690_init();
}

void power_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;
    
    // enable APB1 peripheral clock for I2C1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    // enable clock for SCL and SDA pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    /* setup SCL and SDA pins
        * You can connect I2C1 to two different
        * pairs of pins:
        * 1. SCL on PB6 and SDA on PB7 
        * 2. SCL on PB8 and SDA on PB9
        */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9; // we are going to use PB6 and PB7
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
    GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB
    
    // Connect I2C1 pins to AF  
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1); // SDA
    
    // configure I2C1 
    I2C_InitStruct.I2C_ClockSpeed = 100000; 		// 100kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
    I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
    I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1
    
    // enable I2C1
    I2C_Cmd(I2C1, ENABLE);
}

void max14690_init(void)
{
    /*
     *
     * These were set in bootloader. at some point.
     * TODO
     GPIO_SetBits(GPIOD, GPIO_Pin_2);
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    
    GPIO_SetBits(GPIOF, GPIO_Pin_3);
    GPIO_SetBits(GPIOF, GPIO_Pin_2);
    */
    // reset the chip
    //
    
    // send buck 2 init
    // first send mode for buck 2 for burst
//     I2C_Start(I2C1, 0x50 << 1, I2C_Direction_Transmitter);
//     I2C_Write(I2C1, REG_BUCK2_CFG); // cmd REG_BUCK2_CFG
//     I2C_Write(I2C1, buckCfgVal); // cmd REG_BUCK2_CFG value
//     I2C_Stop(I2C1);
    return;
    // set VREG to 5000 (5v)
    //uint16_t vset = 5000;
    I2C_start(I2C1, 0x50, I2C_Direction_Transmitter);
    I2C_write(I2C1, 0x16); // 
    uint8_t a = I2C_read_ack(I2C1);
    I2C_stop(I2C1);
    
    I2C_start(I2C1, 0x50, I2C_Direction_Transmitter);
    I2C_write(I2C1, 0x16); // 
    I2C_write(I2C1, a); // 
    I2C_stop(I2C1);
    
    I2C_start(I2C1, 0x50, I2C_Direction_Transmitter);
    I2C_write(I2C1, 0x14); // 
    I2C_write(I2C1, 0x03); // 
    I2C_stop(I2C1);
    
    // Set stayOn bit after other registers to confirm successful boot.
//     data = (pfnResEna << 7) |
//            (stayOn);
//     if (writeReg(REG_PWR_CFG, data) != MAX14690_NO_ERROR) {
//         return MAX14690_ERROR;
//     }
    
}


void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction)
{
    // wait until I2C1 is not busy anymore
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

    // Send I2C1 START condition 
    I2C_GenerateSTART(I2Cx, ENABLE);
        
    // wait for I2C1 EV5 --> Slave has acknowledged start condition
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
            
    // Send slave Address for write 
    I2C_Send7bitAddress(I2Cx, address, direction);
        
    /* wait for I2C1 EV6, check if 
        * either Slave has acknowledged Master transmitter or
        * Master receiver mode, depending on the transmission
        * direction
        */ 
    if(direction == I2C_Direction_Transmitter){
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    }
    else if(direction == I2C_Direction_Receiver){
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1 
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
    I2C_SendData(I2Cx, data);
    // wait for I2C1 EV8_2 --> byte has been transmitted
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx)
{
    // Send I2C1 STOP Condition 
    I2C_GenerateSTOP(I2Cx, ENABLE);
}

uint8_t I2C_read_ack(I2C_TypeDef* I2Cx)
{
    // enable acknowledge of recieved data
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    // wait until one byte has been received
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    // read data from I2C data register and return data byte
    uint8_t data = I2C_ReceiveData(I2Cx);
    return data;
}

