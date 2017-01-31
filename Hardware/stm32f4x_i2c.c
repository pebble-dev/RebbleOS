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
#include "stdio.h"
#include "string.h"
#include "stm32f4xx.h"
#include <stm32f4xx_i2c.h>
#include "stm32f4x_i2c.h"


void I2C_init(I2C_conf_t *I2C_conf)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef   I2C_InitStructure;

    // Enable the i2c bus
    RCC_APB1PeriphClockCmd(I2C_conf->RCC_APB1Periph_I2Cx, ENABLE);
    // Reset the I2C Peripheral
    RCC_APB1PeriphResetCmd(I2C_conf->RCC_APB1Periph_I2Cx, ENABLE);
    RCC_APB1PeriphResetCmd(I2C_conf->RCC_APB1Periph_I2Cx, DISABLE);

    //Enable the GPIOs for the SCL/SDA
    RCC_AHB1PeriphClockCmd(I2C_conf->RCC_AHB1Periph_GPIO_SCL | I2C_conf->RCC_AHB1Periph_GPIO_SDA, ENABLE);

    // Configure and initialize the GPIOs
    GPIO_InitStructure.GPIO_Pin = I2C_conf->GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(I2C_conf->GPIO_SCL, &GPIO_InitStructure);
    
    // SDA
    GPIO_InitStructure.GPIO_Pin = I2C_conf->GPIO_Pin_SDA;
    GPIO_Init(I2C_conf->GPIO_SDA, &GPIO_InitStructure);

    // Connect GPIO pins to peripheral
    GPIO_PinAFConfig(I2C_conf->GPIO_SCL, I2C_conf->GPIO_PinSource_SCL, I2C_conf->GPIO_AF_I2Cx);
    GPIO_PinAFConfig(I2C_conf->GPIO_SDA, I2C_conf->GPIO_PinSource_SDA, I2C_conf->GPIO_AF_I2Cx);

    // Configure and Initialize the I2C
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00; //We are the master. We don't need this
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 50000;  //400kHz (Fast Mode) (

    // Initialize the I2C Peripheral
    I2C_Init(I2C_conf->I2Cx, &I2C_InitStructure);
    
    // I2C Peripheral Enable
    I2C_Cmd(I2C_conf->I2Cx, ENABLE);

    return; 
}

void I2C_deinit(I2C_conf_t *I2C_conf)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 

    // I2C Peripheral Disable
    I2C_Cmd(I2C_conf->I2Cx, DISABLE);

    // Disable clock
    I2C_DeInit(I2C_conf->I2Cx);

    // GPIO configuration
    GPIO_InitStructure.GPIO_Pin = I2C_conf->GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(I2C_conf->GPIO_SCL, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = I2C_conf->GPIO_Pin_SDA;
    GPIO_Init(I2C_conf->GPIO_SDA, &GPIO_InitStructure);

    return;
}

// write a byte
void I2C_write_reg(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t reg, uint8_t data)
{  
    I2C_start(I2C_conf);
    
    // Send I2C device Address and clear ADDR
    I2C_addr(I2C_conf, addr, I2C_Direction_Transmitter);
    (void)I2C_conf->I2Cx->SR2;
    
    // Send Data
    I2C_conf->I2Cx->DR = data;
    
    // wait until send busy clears
    I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);
    
    // Generate Stop
    I2C_conf->I2Cx->CR1 |= I2C_CR1_STOP;
    
    I2C_wait_idle(I2C_conf);
    
    return;
}

void I2C_read_reg(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t cnt)
{
    // Send the Register Address
    I2C_start(I2C_conf);
    printf("****I2C 1**\n");
    I2C_addr(I2C_conf, addr, I2C_Direction_Transmitter);
    (void)I2C_conf->I2Cx->SR2;
    printf("****I2C 2**\n");
    I2C_conf->I2Cx->DR = reg;
    
    I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);
    printf("****I2C 3**\n");
    // read
    I2C_read_bytes(I2C_conf, addr, buf, cnt);
    printf("****I2C 4**\n");
    return;
}

uint32_t I2C_write_bytes(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t *buf, uint16_t cnt)
{
    I2C_start(I2C_conf);
    I2C_addr(I2C_conf, addr, I2C_Direction_Transmitter);
    
    // force the clock to stop stretching
    (void)I2C_conf->I2Cx->SR2; 
    
    // write the byte out
    while (cnt--)
    {
        I2C_write_byte(I2C_conf, *buf++);
    }
    
    // wait till complete
    I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);

    // stop when complete, or if stretching, right now.
    I2C_conf->I2Cx->CR1 |= I2C_CR1_STOP;
        
    I2C_wait_idle(I2C_conf);
    
    return 0;
}

uint32_t I2C_read_bytes(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t *buf, uint16_t cnt)
{
    I2C_start(I2C_conf);
  
    // Send I2C Device Address
    I2C_addr(I2C_conf, addr, I2C_Direction_Receiver);
  
    if (cnt == 1)
    {
        // single byte read
        // Disable ACK
        I2C_conf->I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
        // Read SR2 to clear ADDR
        (void)I2C_conf->I2Cx->SR2;
    
        // Send stop. Make sure to send stop after clearing Addr (p583)
        // if not it will cancel the clock being stretched
        I2C_conf->I2Cx->CR1 |= I2C_CR1_STOP;

        I2C_read_byte(I2C_conf, buf);
    
        I2C_wait_idle(I2C_conf);
    
        // Re-enable Ack
        I2C_conf->I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
    }
    else if (cnt==2)
    {  
        // 2 bytes. p584 has a lot to say about this. We need to make sure:
        // that before we reset addr, we set pos and clear ack
        I2C_conf->I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
        I2C_conf->I2Cx->CR1 |= I2C_CR1_POS;
    
        // Hitting SR2 clears addr
        (void)I2C_conf->I2Cx->SR2;
    
        // wait for two bytes. Spec says one will be in DR, and the next will already be clocked in
        // ready to go
        I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);
        
        // We are steching at this point...
        
        // Send stop. We are stetching now as we have all byes. Send immediate stop
        I2C_conf->I2Cx->CR1 |= I2C_CR1_STOP;
        
        // grab 2 bytes from the bus
        I2C_read_byte(I2C_conf, buf++);
        I2C_read_byte(I2C_conf, buf);
        
        I2C_wait_idle(I2C_conf);
        
        // Enable the ack and reset the Pos
        I2C_conf->I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
        I2C_conf->I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_POS);
    }
    else
    {
        // > 2 bytes. p585 says we have to do more work with nack on last byte
        // clear addr
        (void)I2C_conf->I2Cx->SR2;
     
        while((cnt--) > 3)
        {
            I2C_read_byte(I2C_conf, buf++);
        }
    
        // MMMkay so we have 3 bytes remaining at this point.
        I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);
        
        // stretching. We should have the bytes int he buffer now with one left to go
    
        // Reset Ack
        I2C_conf->I2Cx->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
        // reading 3rd byte
        I2C_read_byte(I2C_conf, buf++);
        
        // now we have n3 in DR, n2 in the shit reg. n1 lefy
    
        I2C_wait_for_flags(I2C_conf, I2C_SR1_BTF);
        
        // still stretching
    
        // we can send stop now. We send here before the last byte is shifted in
        // this way we can nack properly the last byte
        I2C_conf->I2Cx->CR1 |= I2C_CR1_STOP;
    
        // Pull n2 into the DR from the shift
        I2C_read_byte(I2C_conf, buf++);
        I2C_read_byte(I2C_conf, buf);
    
        I2C_wait_idle(I2C_conf);
    
        // Enable ack
        I2C_conf->I2Cx->CR1 |= ((uint16_t)I2C_CR1_ACK);
    }
  
    return 0;
}

uint32_t I2C_read_byte(I2C_conf_t *I2C_conf, uint8_t *buf)
{
    uint32_t err;
    
    // Wait for RXNE
    err = I2C_wait_for_flags(I2C_conf, I2C_SR1_RXNE);
        
    if (!err)
    {
        *buf = I2C_conf->I2Cx->DR;
        return 0;
    }
    else
    {
        return err;
    }
}

uint32_t I2C_write_byte(I2C_conf_t *I2C_conf, uint8_t byte)
{ 
    // Write the byte to the DR
    I2C_conf->I2Cx->DR = byte;
  
    return I2C_wait_for_flags(I2C_conf, I2C_SR1_TXE);
}

uint32_t I2C_addr(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t dir)
{
    if (dir == 1)
        addr = addr | 1;
        
    // Send the address to the device. (<<1 for read)
    I2C_conf->I2Cx->DR = addr; //(addr << 1) | dir;
  
    return I2C_wait_for_flags(I2C_conf, I2C_SR1_ADDR);
}

uint32_t I2C_start(I2C_conf_t *I2C_conf)
{  
    I2C_conf->I2Cx->CR1 |= I2C_CR1_START;
  
    // start bit set, clock stretching.
    // wait for start bit completion
    return I2C_wait_for_flags(I2C_conf, I2C_SR1_SB);
}

uint8_t I2C_wait_for_flags(I2C_conf_t *I2C_conf, uint32_t Flags)
{
    uint16_t timeout = 5000;
  
    while(((I2C_conf->I2Cx->SR1) & Flags) != Flags)
    {
        if (!(timeout--))
        {
            printf("Timeout I2C\n");
            return 1;
        }
    }
    return 0;
}

uint8_t I2C_wait_idle(I2C_conf_t *I2C_conf)
{ 
    uint32_t timeout = 5000;

    while((I2C_conf->I2Cx->SR2) & (I2C_SR2_BUSY))
    {
        if (!(timeout--))
        {
            printf("i2C Timeout on Idle\n");
            return 1;
        }
    }
    
    return 0;
}


