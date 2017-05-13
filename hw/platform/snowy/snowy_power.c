/* snowy_power.c
 * MAX14690 PMIC management routines for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "snowy_power.h"
#include <stm32f4xx_spi.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_tim.h>
#include "stm32f4x_i2c.h"
#include "stm32_power.h"

// useful?
// https://developer.mbed.org/users/switches/code/MAX14690/file/666b6c505289/MAX14690.h

// massive work in progress

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

I2C_conf_t I2C_conf = {
    .I2Cx                      = I2C1,
    .RCC_APB1Periph_I2Cx       = RCC_APB1Periph_I2C1,
    .RCC_AHB1Periph_GPIO_SCL   = RCC_AHB1Periph_GPIOB,
    .RCC_AHB1Periph_GPIO_SDA   = RCC_AHB1Periph_GPIOB,
    .GPIO_AF_I2Cx              = GPIO_AF_I2C1,
    .GPIO_SCL                  = GPIOB,
    .GPIO_SDA                  = GPIOB,
    .GPIO_Pin_SCL              = GPIO_Pin_6,
    .GPIO_Pin_SDA              = GPIO_Pin_9,
    .GPIO_PinSource_SCL        = GPIO_PinSource6,
    .GPIO_PinSource_SDA        = GPIO_PinSource9,
};

void hw_power_init(void)
{   
    I2C_init(&I2C_conf);
    max14690_init();
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
    GPIO_InitTypeDef GPIO_InitStructure;
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOF);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOF, GPIO_Pin_3);
    GPIO_SetBits(GPIOF, GPIO_Pin_2);

    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOF);
    
    uint8_t buf[0x1F];
    I2C_read_reg(&I2C_conf, 0x50, 0x00, buf, 0x1F + 1);
    
//     for (uint16_t i = 0; i < 0x1F + 1; i++)
//     {
//         printf("R: %02x %02x \n", i, buf[i]);
//     }
    
    // En LDO2
    I2C_write_reg(&I2C_conf, 0x50, 0x14, 0x03);

    buf[0] = 0x3;
    // En LDO3
    I2C_write_reg(&I2C_conf, 0x50, 0x16, 0x03);
//     printf("****I2C 0x16**\n");    
}
