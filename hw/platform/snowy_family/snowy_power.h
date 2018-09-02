/* snowy_power.h
 * MAX14690 PMIC management routines for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

/* Monitor register */
#define MON_RATIO_4_1 0
#define MON_RATIO_3_1 1
#define MON_RATIO_2_1 2
#define MON_RATIO_1_1 3

#define MON_OFF_MODE_PD_100K 0
#define MON_OFF_MODE_HIZ     1

#define MON_CTRL_HIZ     0
#define MON_CTRL_BATT    1
#define MON_CTRL_SYS     2
#define MON_CTRL_BUCK1   3
#define MON_CTRL_BUCK2   4
#define MON_CTRL_LDO1    5
#define MON_CTRL_LDO2    6
#define MON_CTRL_LDO3    7


/* MAX14690 register definitions */
#define REG_STATUS_A     0x02
#define REG_STATUS_B     0x03
#define REG_BUCK2_CFG    0x0F
#define REG_BUCK2_VSET   0x10
#define REG_MON_CFG      0x19
#define REG_PWR_CFG      0x1D

typedef struct {
    uint8_t address;   
    uint16_t pin_intn;   // power interrupt
} max14690_t;

void hw_power_init(void);
uint16_t hw_power_get_bat_mv(void);
uint8_t hw_power_get_chg_status(void);

/**
 * @brief command the max14690 to stay enabled after powerup mode
 * @param pullup when > 0 will enable the internal 100k pullup
 */
void max14690_stay_on(uint8_t pullup);

/**
 * @brief Set the status of the MON pin on the max14690.
 * The register contains a bit set voltage divider and internal measure source
 * @param control measurement source for the MON output. 0 is off
 * @param mode When MON is disabled, set the MON to HiZ or pulldown
 * @param ratio internal voltage divider ratio
 */
void max14690_set_monitor_status(uint8_t control, uint8_t mode, uint8_t ratio);
