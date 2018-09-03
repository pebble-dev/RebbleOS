#pragma once
/* stm32f4x_i2c.h
 * STM I2c wrapper driver
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

/**
 * @brief structure defining the settings for an I2C connection
 */
typedef struct
{
    I2C_TypeDef *i2c_x;
    uint32_t i2c_clock;
    uint32_t gpio_clock;
    uint8_t gpio_af;
    GPIO_TypeDef * gpio_port;
    uint16_t gpio_pin_scl;
    uint16_t gpio_pin_sda;
    uint8_t gpio_pinsource_scl;
    uint8_t gpio_pinsource_sda;
} stm32_i2c_conf_t;

/**
 * @brief initialise i2c module
 * @param i2c_conf @ref stm_i2c_conf_t struct
 */
void i2c_init(const stm32_i2c_conf_t *i2c_conf);

/**
 * @brief de-initialise i2c module
 * @param i2c_conf @ref stm_i2c_conf_t struct
 */
void i2c_deinit(const stm32_i2c_conf_t *i2c_conf);

/**
 * @brief Write a register with a given value
 * @param i2c_conf @ref stm_i2c_conf_t struct
 * @param addr address of the device (<<1 shift)
 * @param reg register address to write the data to
 * @param data the value to write to the register
 *
 * @return number of read bytes
 */
uint8_t i2c_write_reg(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief Write a byte stream to the device
 * @param i2c_conf @ref stm_i2c_conf_t struct
 * @param addr address of the device (<<1 shift)
 * @param buf The buffer pointer to write to the device
 * @param cnt the number of bytes to write
 * 
 * @return number of written bytes
 */
uint32_t i2c_write_bytes(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t *buf, uint16_t cnt);

/**
 * @brief Read a given register on the bus with n count
 * @param i2c_conf @ref stm_i2c_conf_t struct
 * @param addr address of the device (<<1 shift)
 * @param buf The buffer pointer to write to the device
 * @param cnt the number of bytes to write
 * 
 * @return number of read bytes
 */
uint8_t i2c_read_reg(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t cnt);

/**
 * @brief Write an addressing register, then restart to read n bytes
 * @param i2c_conf @ref stm_i2c_conf_t struct
 * @param addr address of the device (<<1 shift)
 * @param buf The buffer pointer to write to the device
 * @param cnt the number of bytes to write
 * 
 * @return number of read bytes
 */
uint32_t i2c_read_bytes(const stm32_i2c_conf_t *i2c_conf, uint8_t addr, uint8_t *buf, uint16_t cnt);
