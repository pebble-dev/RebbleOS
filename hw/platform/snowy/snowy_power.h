/* snowy_power.h
 * MAX14690 PMIC management routines for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
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
