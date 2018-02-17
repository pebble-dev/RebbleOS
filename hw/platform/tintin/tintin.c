/* tintin.c
 * miscellaneous routines for tintin-like devices
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stm32f2xx.h>
#include "tintin.h"
#include <debug.h>

#include <stm32f2xx_usart.h>
#include <stm32f2xx_gpio.h>
#include <stm32f2xx_spi.h>
#include <stm32f2xx_iwdg.h>
#include <stm32f2xx_rcc.h>
#include <stm32f2xx_syscfg.h>
#include <misc.h>

#include "stm32_power.h"
#include "stm32_rtc.h"

extern void *strcpy(char *a2, const char *a1);

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

    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_USART3);
    
    for (i = 0; i < len; i++) {
        while (!(USART3->SR & USART_SR_TC));
        USART3->DR = p[i];
    }

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_USART3);
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
static void _init_USART3(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_USART3);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);

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

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_USART3);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
}

/*** platform ***/

void platform_init() {
    stm32_power_init();
    
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
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(0xFFF);
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
    IWDG_Enable();
    IWDG_ReloadCounter();
}

void hw_watchdog_reset() {
    /* I don't think so, homeslice. */
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

    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_SPI2);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);

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

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_SPI2);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
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
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_SPI2);

    printf("tintin: here we go, slowly blitting %d %d\n", x, y);
    GPIO_WriteBit(GPIOB, 1 << 12, 1);
    delay_us(7);
    _display_write(0x80);
    for (int i = 0; i < 168; i++) {
        _display_write(__RBIT(__REV(167-i)));
        for (int j = 0; j < 18; j++)
            _display_write(_display_fb[i][17-j]);
        _display_write(0);
    }
    _display_write(0);
    delay_us(7);
    GPIO_WriteBit(GPIOB, 1 << 12, 0);

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_SPI2);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);

    display_done_ISR(0);
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

/* vibrate */

void hw_vibrate_init() {
}

void hw_vibrate_enable(uint8_t enabled) {
}


#define JEDEC_READ 0x03
#define JEDEC_RDSR 0x05
#define JEDEC_IDCODE 0x9F
#define JEDEC_DUMMY 0xA9
#define JEDEC_WAKE 0xAB
#define JEDEC_SLEEP 0xB9

#define JEDEC_RDSR_BUSY 0x01

#define JEDEC_IDCODE_MICRON_N25Q032A11 0x20BB16

static uint8_t _hw_flash_txrx(uint8_t c) {
    while (!(SPI1->SR & SPI_SR_TXE))
        ;
    SPI1->DR = c;
    while (!(SPI1->SR & SPI_SR_RXNE))
        ;
    return SPI1->DR;
}

static void _hw_flash_enable(int i) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    GPIO_WriteBit(GPIOA, 1 << 4, !i);
    delay_us(1);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
}

static void _hw_flash_wfidle() {
    _hw_flash_enable(1);
    _hw_flash_txrx(JEDEC_RDSR);
    while (_hw_flash_txrx(JEDEC_DUMMY) & JEDEC_RDSR_BUSY)
        ;
    _hw_flash_enable(0);
}


void hw_flash_init(void) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);
    
    /* Set up the pins. */
    GPIO_WriteBit(GPIOA, 1 << 4, 0); /* nCS */
    GPIO_PinAFConfig(GPIOA, 5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, 6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, 7, GPIO_AF_SPI1);
    
    GPIO_InitTypeDef gpioinit;
    
    gpioinit.GPIO_Pin = (1 << 7) | (1 << 6);
    gpioinit.GPIO_Mode = GPIO_Mode_AF;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpioinit);

    gpioinit.GPIO_Pin = (1 << 5);
    gpioinit.GPIO_Mode = GPIO_Mode_AF;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &gpioinit);

    gpioinit.GPIO_Pin = (1 << 4);
    gpioinit.GPIO_Mode = GPIO_Mode_OUT;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioinit.GPIO_OType = GPIO_OType_PP;
    gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpioinit);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOA);

    /* Set up the SPI controller, SPI1. */
    SPI_InitTypeDef spiinit;
    
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI1);

    SPI_I2S_DeInit(SPI2);
    
    spiinit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spiinit.SPI_Mode = SPI_Mode_Master;
    spiinit.SPI_DataSize = SPI_DataSize_8b;
    spiinit.SPI_CPOL = SPI_CPOL_Low;
    spiinit.SPI_CPHA = SPI_CPHA_1Edge;
    spiinit.SPI_NSS = SPI_NSS_Soft;
    spiinit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spiinit.SPI_FirstBit = SPI_FirstBit_MSB;
    spiinit.SPI_CRCPolynomial = 7 /* Um. */;
    SPI_Init(SPI2, &spiinit);
    SPI_Cmd(SPI2, ENABLE);
    
    /* In theory, SPI is up.  Now let's see if we can talk to the part. */
    _hw_flash_enable(1);
    _hw_flash_txrx(JEDEC_WAKE);
    _hw_flash_enable(0);
    delay_us(100);
    
    _hw_flash_wfidle();
     
    uint32_t part_id = 0;
    
    _hw_flash_enable(1);
    _hw_flash_txrx(JEDEC_IDCODE);
    part_id |= _hw_flash_txrx(JEDEC_DUMMY) << 16;
    part_id |= _hw_flash_txrx(JEDEC_DUMMY) << 8;
    part_id |= _hw_flash_txrx(JEDEC_DUMMY) << 0;
    _hw_flash_enable(0);
    
    printf("tintin flash: JEDEC ID %08lx\n", part_id);
    
    if (part_id != JEDEC_IDCODE_MICRON_N25Q032A11) {
        panic("tintin flash: unsupported part ID");
    }
    
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI1);

}


void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    assert(addr < 0x1000000 && "address too large for JEDEC_READ command");
    
    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SPI1);

    _hw_flash_wfidle();
    
    _hw_flash_enable(1);
    _hw_flash_txrx(JEDEC_READ);
    _hw_flash_txrx((addr >> 16) & 0xFF);
    _hw_flash_txrx((addr >>  8) & 0xFF);
    _hw_flash_txrx((addr >>  0) & 0xFF);
    
    /* XXX: could use DMA, conceivably */
    for (int i = 0; i < len; i++)
        buf[i] = _hw_flash_txrx(JEDEC_DUMMY);
    
    _hw_flash_enable(0);
    
    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SPI1);
}

void ss_debug_write(const unsigned char *p, size_t len)
{
    // unsupported on this platform
}

void log_clock_enable() {
}
void log_clock_disable() {
}

void HardFault_Handler()
{
    printf("*** HARD FAULT ***\n");
    while(1);
}

void BusFault_Handler()
{
    printf("*** BUS FAULT ***\n");
    while(1);
}

void UsageFault_Handler_C(uint32_t *sp)
{
    uint16_t ufsr = *(uint16_t *)0xE000ED2A;
    
    printf("*** USAGE FAULT ***\n");
    printf("   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx\n", sp[4], sp[5], sp[6], (uint32_t) sp);
    printf("  UFSR: %04x\n", ufsr);
    
    if (ufsr & 1) {
        printf("    *PC == %04x\n", *(uint16_t *)sp[6]);
    }
    while(1);
}

__attribute__((naked)) void UsageFault_Handler()
{
    asm volatile (
        "TST   LR, #4\n\t"
        "ITE   EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"
        "LDR   R1, =UsageFault_Handler_C\n\t"
        "BX    R1"
    );
}
