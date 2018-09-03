/* stm32f4x_i2c.c
 * STM I2c wrapper driver
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#    include "stm32f4xx_i2c.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_i2c.h"
#    include "stm32f2xx_rcc.h"
#    include "stm32f2xx_exti.h"
#    include "stm32f2xx_pwr.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include "stdio.h"
#include "string.h"
#include "stm32_power.h"
#include "stm32_rtc.h"
#include "log.h"
#include "stm32_i2c.h"

static uint32_t _i2c_write_byte(const stm32_i2c_conf_t *i2c_conf, uint8_t byte);
static uint32_t _i2c_read_byte(const stm32_i2c_conf_t *i2c_conf, uint8_t *buf);
static uint32_t _i2c_addr(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t dir);
static uint32_t _i2c_start(const stm32_i2c_conf_t *i2c_conf);
static uint8_t _i2c_wait_for_flags(const stm32_i2c_conf_t *i2c_conf, uint32_t Flags);
static uint8_t _i2c_wait_idle(const stm32_i2c_conf_t *i2c_conf);

void i2c_init(const stm32_i2c_conf_t *i2c_conf)
{
    GPIO_InitTypeDef  gpio_initstruct;
    I2C_InitTypeDef   i2c_initstruct;

    stm32_power_request(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_request(STM32_POWER_AHB1, i2c_conf->gpio_clock);

    /* Reset the I2C Peripheral */
    RCC_APB1PeriphResetCmd(i2c_conf->i2c_clock, ENABLE);
    RCC_APB1PeriphResetCmd(i2c_conf->i2c_clock, DISABLE);

    RCC_AHB1PeriphClockCmd(i2c_conf->gpio_clock, ENABLE);

    /* Configure and initialize the GPIOs. SCL */
    gpio_initstruct.GPIO_Pin = i2c_conf->gpio_pin_scl;
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_initstruct.GPIO_OType = GPIO_OType_OD;
    gpio_initstruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(i2c_conf->gpio_port, &gpio_initstruct);    
    
    /* SDA */
    gpio_initstruct.GPIO_Pin = i2c_conf->gpio_pin_sda;
    GPIO_Init(i2c_conf->gpio_port, &gpio_initstruct);

    /* Connect GPIO pins to peripheral */
    GPIO_PinAFConfig(i2c_conf->gpio_port, i2c_conf->gpio_pinsource_scl, i2c_conf->gpio_af);
    GPIO_PinAFConfig(i2c_conf->gpio_port, i2c_conf->gpio_pinsource_sda, i2c_conf->gpio_af);
 
    /* Configure and Initialize the I2C */
    i2c_initstruct.I2C_Mode = I2C_Mode_I2C;
    i2c_initstruct.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c_initstruct.I2C_OwnAddress1 = 0x00; //We are the master. We don't need this
    i2c_initstruct.I2C_Ack = I2C_Ack_Enable;
    i2c_initstruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c_initstruct.I2C_ClockSpeed = 400000;  //400kHz (Fast Mode)

    /* Initialize the I2C Peripheral */
    I2C_Init(i2c_conf->i2c_x, &i2c_initstruct);
    
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c_conf->i2c_x, ENABLE);

    stm32_power_release(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_release(STM32_POWER_AHB1, i2c_conf->gpio_clock);
}

void i2c_deinit(const stm32_i2c_conf_t *i2c_conf)
{
    GPIO_InitTypeDef  gpio_initstruct; 

    stm32_power_request(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_request(STM32_POWER_AHB1, i2c_conf->gpio_clock);

    /* Deinit i2c and the clocks */
    I2C_Cmd(i2c_conf->i2c_x, DISABLE);
    I2C_DeInit(i2c_conf->i2c_x);

    /* Set GPIO to inputs */
    gpio_initstruct.GPIO_Pin = i2c_conf->gpio_pin_scl;
    gpio_initstruct.GPIO_Mode = GPIO_Mode_IN;
    gpio_initstruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(i2c_conf->gpio_port, &gpio_initstruct);

    gpio_initstruct.GPIO_Pin = i2c_conf->gpio_pin_sda;
    GPIO_Init(i2c_conf->gpio_port, &gpio_initstruct);

    stm32_power_release(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_release(STM32_POWER_AHB1, i2c_conf->gpio_clock);
}

uint8_t i2c_write_reg(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    
    stm32_power_request(STM32_POWER_AHB1, i2c_conf->gpio_clock);
    stm32_power_request(STM32_POWER_APB1, i2c_conf->i2c_clock);
    
    i2c_write_bytes(i2c_conf, addr, buf, 2);
    
    stm32_power_release(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_release(STM32_POWER_AHB1, i2c_conf->gpio_clock);
    
    return 1;
}

uint8_t i2c_read_reg(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t cnt)
{
    stm32_power_request(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_request(STM32_POWER_AHB1, i2c_conf->gpio_clock);

    if (_i2c_start(i2c_conf))
        return 0;

    if (_i2c_addr(i2c_conf, addr, I2C_Direction_Transmitter))
        return 0;
    
    /* force the clock to stop stretching */
    (void)i2c_conf->i2c_x->SR2;

    i2c_conf->i2c_x->DR = reg;
    
    _i2c_wait_for_flags(i2c_conf, I2C_SR1_BTF);

    /* Switch to read mode and pull the bytes into the buf */
    i2c_read_bytes(i2c_conf, addr, buf, cnt);

    stm32_power_release(STM32_POWER_APB1, i2c_conf->i2c_clock);
    stm32_power_release(STM32_POWER_AHB1, i2c_conf->gpio_clock);

    return cnt;
}

uint32_t i2c_write_bytes(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t *buf, uint16_t cnt)
{
    if (_i2c_start(i2c_conf))
        return 0;
    
    if (_i2c_addr(i2c_conf, addr, I2C_Direction_Transmitter))
        return 0;
    
    /* force the clock to stop stretching */
    (void)i2c_conf->i2c_x->SR2; 
    
    /* write the bytes out */
    while (cnt--)
    {
        _i2c_write_byte(i2c_conf, *buf++);
    }

    /* wait till complete */
    _i2c_wait_for_flags(i2c_conf, I2C_SR1_BTF);

    /* stop when complete, or if stretching, right now. */
    i2c_conf->i2c_x->CR1 |= I2C_CR1_STOP;
    
    _i2c_wait_idle(i2c_conf);    
    
    return cnt;
}

uint32_t i2c_read_bytes(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t *buf, uint16_t cnt)
{
    if (_i2c_start(i2c_conf))
        return 0;
  
    /* Send I2C device address */
    if (_i2c_addr(i2c_conf, addr, I2C_Direction_Receiver))
        return 0;
  
    if (cnt == 1)
    {
        /* single byte read. Disable ACK */
        i2c_conf->i2c_x->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
        /* Read SR2 to clear ADDR */
        (void)i2c_conf->i2c_x->SR2;

        /* Send stop. Make sure to send stop after clearing Addr (p583)
         * if not it will cancel the clock being stretched */
        i2c_conf->i2c_x->CR1 |= I2C_CR1_STOP;

        _i2c_read_byte(i2c_conf, buf);
        _i2c_wait_idle(i2c_conf);

        /* Re-enable Ack */
        i2c_conf->i2c_x->CR1 |= ((uint16_t)I2C_CR1_ACK);
    }
    else if (cnt==2)
    {
        /* 2 bytes. p584 has a lot to say about this. We need to make sure:
         * that before we reset addr, we set pos and clear ack */
        i2c_conf->i2c_x->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
        i2c_conf->i2c_x->CR1 |= I2C_CR1_POS;
    
        /* Hitting SR2 clears addr */
        (void)i2c_conf->i2c_x->SR2;
    
        /* wait for two bytes. Spec says one will be in DR, and the next 
         * will already be clocked in ready to go */
        _i2c_wait_for_flags(i2c_conf, I2C_SR1_BTF);
        
        /* We are steching at this point...         
         * Send stop. We are stetching now as we have all byes. 
         * Send immediate stop */
        i2c_conf->i2c_x->CR1 |= I2C_CR1_STOP;
        
        /* grab 2 bytes from the bus */
        _i2c_read_byte(i2c_conf, buf++);
        _i2c_read_byte(i2c_conf, buf);
        
        _i2c_wait_idle(i2c_conf);
        
        /* Enable the ack and reset the Pos */
        i2c_conf->i2c_x->CR1 |= ((uint16_t)I2C_CR1_ACK);
        i2c_conf->i2c_x->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_POS);
    }
    else
    {
        /* > 2 bytes. p585 says we have to do more work with nack on last byte
         * clear addr */
        (void)i2c_conf->i2c_x->SR2;
     
        while((cnt--) > 3)
        {
            _i2c_read_byte(i2c_conf, buf++);
        }
    
        /* MMMkay so we have 3 bytes remaining at this point. */
        _i2c_wait_for_flags(i2c_conf, I2C_SR1_BTF);
        
        /* stretching. We should have the bytes int he buffer now with one left to go    
         * Reset Ack */
        i2c_conf->i2c_x->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
    
        /* reading 3rd byte */
        _i2c_read_byte(i2c_conf, buf++);
        
        /* now we have n3 in DR, n2 in the shift reg. n1 left */    
        _i2c_wait_for_flags(i2c_conf, I2C_SR1_BTF);
        
        /* still stretching    
         * we can send stop now. We send here before the last byte is shifted in
         * this way we can nack properly the last byte */
        i2c_conf->i2c_x->CR1 |= I2C_CR1_STOP;
    
        /* Pull n2 into the DR from the shift */
        _i2c_read_byte(i2c_conf, buf++);
        _i2c_read_byte(i2c_conf, buf);
    
        _i2c_wait_idle(i2c_conf);
    
        /* Enable ack */
        i2c_conf->i2c_x->CR1 |= ((uint16_t)I2C_CR1_ACK);
    }

    return cnt;
}

static uint32_t _i2c_read_byte(const stm32_i2c_conf_t *i2c_conf, uint8_t *buf)
{
    uint32_t err;

    /* Wait for RXNE */
    err = _i2c_wait_for_flags(i2c_conf, I2C_SR1_RXNE);
        
    if (!err)
    {
        *buf = i2c_conf->i2c_x->DR;
        return 0;
    }
    else
    {
        return err;
    }
}

static uint32_t _i2c_write_byte(const stm32_i2c_conf_t *i2c_conf, uint8_t byte)
{ 
    i2c_conf->i2c_x->DR = byte;
  
    return _i2c_wait_for_flags(i2c_conf, I2C_SR1_TXE);
}

static uint32_t _i2c_addr(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t dir)
{       
    /* Send the address to the device. ( | 1 for read) */
    i2c_conf->i2c_x->DR = (addr << 1) | dir;
  
    return _i2c_wait_for_flags(i2c_conf, I2C_SR1_ADDR);
}

static uint32_t _i2c_start(const stm32_i2c_conf_t *i2c_conf)
{  
    i2c_conf->i2c_x->CR1 |= I2C_CR1_START;
  
    /* start bit set, clock stretching.
     * wait for start bit completion */
    return _i2c_wait_for_flags(i2c_conf, I2C_SR1_SB);
}

static uint8_t _i2c_wait_for_flags(const stm32_i2c_conf_t *i2c_conf, uint32_t flags)
{
    uint16_t timeout = 5000;
    
    if (i2c_conf->i2c_x->SR1 & (I2C_SR1_OVR | I2C_SR1_ARLO | I2C_SR1_BERR))
    {
        DRV_LOG("I2C", APP_LOG_LEVEL_ERROR, "Bus Error");
        return 1;
    }
  
    while(((i2c_conf->i2c_x->SR1) & flags) != flags)
    {
        if (!(timeout--))
        {
            DRV_LOG("I2C", APP_LOG_LEVEL_ERROR, "Timeout on flag %lx", flags);
            return 1;
        }
    }
    return 0;
}

static uint8_t _i2c_wait_idle(const stm32_i2c_conf_t *i2c_conf)
{ 
    uint32_t timeout = 5000;

    while((i2c_conf->i2c_x->SR2) & (I2C_SR2_BUSY))
    {
        if (!(timeout--))
        {
            DRV_LOG("I2C", APP_LOG_LEVEL_ERROR, "Timeout on Idle");
            return 1;
        }
    }
    
    return 0;
}

