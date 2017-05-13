#ifndef __STM32F4x_i2c_H
#define __STM32F4x_i2c_H


typedef struct
{
    I2C_TypeDef *I2Cx;
    uint32_t RCC_APB1Periph_I2Cx;
    uint32_t RCC_AHB1Periph_GPIO_SCL;
    uint32_t RCC_AHB1Periph_GPIO_SDA;
    uint8_t GPIO_AF_I2Cx;
    GPIO_TypeDef * GPIO_SCL;
    GPIO_TypeDef * GPIO_SDA;
    uint16_t GPIO_Pin_SCL;
    uint16_t GPIO_Pin_SDA;
    uint8_t GPIO_PinSource_SCL;
    uint8_t GPIO_PinSource_SDA;
} I2C_conf_t;

void I2C_init(I2C_conf_t *I2C_conf);
void I2C_deinit(I2C_conf_t *I2C_conf);
void I2C_write_reg(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t reg, uint8_t data);
void I2C_read_reg(I2C_conf_t *I2C_conf, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t cnt);

#endif
