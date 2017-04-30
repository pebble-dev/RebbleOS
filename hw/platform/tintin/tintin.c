/* tintin.c
 * miscellaneous routines for tintin-like devices
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stm32f2xx.h>
#include "tintin.h"

#include <stm32f2xx_usart.h>
#include <stm32f2xx_gpio.h>
#include <stm32f2xx_spi.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>

/*** debug routines ***/

static void _init_USART3();
static int _debug_initialized;

void debug_init() {
    _init_USART3();
    _debug_initialized = 1;
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len) {
    int i;

    if (!_debug_initialized)
        return;

    /* XXX: better power management */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    for (i = 0; i < len; i++) {
        while (!(USART3->SR & USART_SR_TC));
        USART3->DR = p[i];
    }
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
static void _init_USART3(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStruct);
    USART_Cmd(USART3, ENABLE);
}

/*** platform ***/

void platform_init() {
    SCB->VTOR = 0x08004000;
    NVIC_SetVectorTable(0x08004000, 0);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void platform_init_late() {
    printf("tintin: late init\n");
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    printf("c     : %lu", SystemCoreClock);
    printf("SYSCLK: %lu\n", RCC_Clocks.SYSCLK_Frequency);
    printf("CFGR  : %lx\n", RCC->PLLCFGR);
}

/*** watchdog timer ***/

void hw_watchdog_init() {
}

void hw_watchdog_reset() {
}

/*** ambient light sensor ***/

void hw_ambient_init() {
}

uint16_t hw_ambient_get() {
    return 0;
}

/*** backlight init ***/

void hw_backlight_init() {
}

void hw_backlight_set(uint16_t val) {
}

/* buttons */

#include "stm32_buttons_platform.h"

stm32_button_t platform_buttons[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK]   = { GPIO_Pin_3, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource3, RCC_AHB1Periph_GPIOC, EXTI3_IRQn },
    [HW_BUTTON_UP]     = { GPIO_Pin_2, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource2, RCC_AHB1Periph_GPIOA, EXTI2_IRQn },
    [HW_BUTTON_SELECT] = { GPIO_Pin_6, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource6, RCC_AHB1Periph_GPIOC, EXTI9_5_IRQn },
    [HW_BUTTON_DOWN]   = { GPIO_Pin_1, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource1, RCC_AHB1Periph_GPIOA, EXTI1_IRQn }
};

STM32_BUTTONS_MK_IRQ_HANDLER(3)
STM32_BUTTONS_MK_IRQ_HANDLER(2)
STM32_BUTTONS_MK_IRQ_HANDLER(9_5)
STM32_BUTTONS_MK_IRQ_HANDLER(1)

/* display */

static uint8_t _display_fb[168][20];

void hw_display_init() {
    printf("tintin: hw_display_init\n");

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    GPIO_WriteBit(GPIOB, 1 << 12, 0);
    GPIO_PinAFConfig(GPIOB, 1, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, 13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, 15, GPIO_AF_SPI2);

    GPIO_InitTypeDef gpioinit;
    
    gpioinit.GPIO_Pin = (1 << 12);
    gpioinit.GPIO_Mode = GPIO_Mode_OUT;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpioinit);

    gpioinit.GPIO_Pin = (1 << 13) | (1 << 15);
    gpioinit.GPIO_Mode = GPIO_Mode_AF;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpioinit);
    
    gpioinit.GPIO_Pin = (1 << 1);
    gpioinit.GPIO_Mode = GPIO_Mode_AF;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_OD;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpioinit);

    /* Set up the SPI controller, SPI2. */
    SPI_InitTypeDef spiinit;
    
    SPI_I2S_DeInit(SPI2);
    spiinit.SPI_Direction = SPI_Direction_1Line_Tx;
    spiinit.SPI_Mode = SPI_Mode_Master;
    spiinit.SPI_DataSize = SPI_DataSize_8b;
    spiinit.SPI_CPOL = SPI_CPOL_Low;
    spiinit.SPI_CPHA = SPI_CPHA_1Edge;
    spiinit.SPI_NSS = SPI_NSS_Soft;
    spiinit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    spiinit.SPI_FirstBit = SPI_FirstBit_MSB;
    spiinit.SPI_CRCPolynomial = 7 /* Um. */;
    SPI_Init(SPI2, &spiinit);
    SPI_Cmd(SPI2, ENABLE);
}

void hw_display_reset() {
    printf("tintin: hw_display_reset\n");
}

void hw_display_start() {
    printf("tintin: hw_display_start\n");
}

static void _display_write(unsigned char c) {
    SPI2->DR = c;
    while (!(SPI2->SR & SPI_SR_TXE))
        ;
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    printf("tintin: here we go, slowly blitting %d %d\n", x, y);
    GPIO_WriteBit(GPIOB, 1 << 12, 1);
    delay_us(7);
    _display_write(0x80);
    for (int i = 0; i < 168; i++) {
        _display_write(__RBIT(__REV(167-i)));
        for (int j = 0; j < 18; j++)
            _display_write(__RBIT(__REV(_display_fb[i][j])));
        _display_write(0);
    }
    _display_write(0);
    delay_us(7);
    GPIO_WriteBit(GPIOB, 1 << 12, 0);
}

uint8_t *hw_display_get_buffer(void) {
    return (uint8_t *)_display_fb;
}

uint8_t hw_display_get_state() {
    return 1;
}


// void scanline_rgb888pixel_to_frambuffer(UG_S16 x, UG_S16 y, UG_COLOR c) {
//     if (c)
//         _display_fb[y][17 - (x / 8)] |= (0x80 >> (x % 8));
//     else
//         _display_fb[y][17 - (x / 8)] &= ~(0x80 >> (x % 8));
// }

/* rtc */

void rtc_init() {
}

void rtc_config() {
}

void hw_get_time_str(char *buf) {
    strcpy(buf, "buttsquid");
}

#include <time.h>
static struct tm _tm;
struct tm *hw_get_time() {
    return &_tm;
}

void rtc_set_timer_interval(TimeUnits tick_units)
{
}

void rtc_disable_timer_interval(void)
{
}
/* vibrate */

void hw_vibrate_init() {
}

void hw_vibrate_enable(uint8_t enabled) {
}


void hw_flash_init() {
}

uint16_t hw_flash_read16(uint32_t address) {
    return 0xC0;   
}

uint32_t hw_flash_read32(uint32_t address) {
    return 0xC0;
}

void hw_flash_read_bytes(uint32_t address, uint8_t *buffer, size_t length) {
    memcpy(buffer, 0xC0, length);
}

void hw_flash_write16(uint32_t address, uint16_t data) {
    
}

void ss_debug_write(const unsigned char *p, size_t len)
{
    // unsupported on this platform
}
