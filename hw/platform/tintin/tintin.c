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
#include "rebbleos.h"

#include "stm32_usart.h"
#include "stm32_power.h"
#include "stm32_rtc.h"
#include "stm32_backlight.h"
#include "stm32_spi.h"

// extern void *strcpy(char *a2, const char *a1);

/*** debug routines ***/

static inline void _init_USART3();
static int _debug_initialized;


static const stm32_usart_config_t _u3_config = {
    .usart                = USART3,
    .flow_control_enabled = FLOW_CONTROL_DISABLED,
    .usart_periph_bus     = STM32_POWER_APB1,
    .gpio_pin_tx_num      = 10,
    .gpio_pin_rx_num      = 11,
    .gpio_pin_rts_num     = 0,
    .gpio_pin_cts_num     = 0,
    .gpio_ptr             = GPIOC,
    .gpio_clock           = RCC_AHB1Periph_GPIOC,
    .usart_clock          = RCC_APB1Periph_USART3,
    .af                   = GPIO_AF_USART3,
};

static stm32_usart_t _usart3 = {
    &_u3_config,
    NULL,
    230400
};


static const stm32_spi_config_t _spi2_config = {
    .spi                  = SPI2,
    .spi_periph_bus       = STM32_POWER_APB1,
    .gpio_pin_miso_num    = 0,
    .gpio_pin_mosi_num    = 15,
    .gpio_pin_sck_num     = 13,
    .gpio_ptr             = GPIOB,
    .gpio_clock           = RCC_AHB1Periph_GPIOB,
    .spi_clock            = RCC_APB1Periph_SPI2,
    .af                   = GPIO_AF_SPI2,
    .txrx_dir             = STM32_SPI_DIR_TX,
    .crc_poly             = 7 /* Um */
};

static const stm32_dma_t _spi2_dma = {
    .dma_clock            = RCC_AHB1Periph_DMA1,
    .dma_tx_stream        = DMA1_Stream4,
    .dma_rx_stream        = DMA1_Stream3,
    .dma_tx_channel       = DMA_Channel_0,
    .dma_rx_channel       = DMA_Channel_0,
    .dma_irq_tx_pri       = 10,
    .dma_irq_rx_pri       = 9,
    .dma_irq_tx_channel   = DMA1_Stream4_IRQn,
    .dma_irq_rx_channel   = DMA1_Stream3_IRQn,
    .dma_tx_channel_flags = STM32_DMA_MK_FLAGS(4),
    .dma_rx_channel_flags = STM32_DMA_MK_FLAGS(3),
    .dma_tx_irq_flag      = DMA_IT_TCIF4,
    .dma_rx_irq_flag      = DMA_IT_TCIF3
};

static stm32_spi_t _spi2 = {
    &_spi2_config,
    &_spi2_dma, /* dma */
};

//STM32_SPI_MK_TX_IRQ_HANDLER(&_spi2, 2, 5, _spi_tx_done)

void debug_init() {
    _init_USART3();
    _debug_initialized = 1;
}

/* note that locking needs to be handled by external entity here */
void debug_write(const unsigned char *p, size_t len) {
    int i;

    if (!_debug_initialized)
        return;

    stm32_usart_write(&_usart3, p, len);
}

/*
 * Configure USART3(PB10, PB11) to redirect printf data to host PC.
 */
static inline void _init_USART3(void)
{
    /* leave the usart clock on */
    stm32_power_request(_usart3.config->usart_periph_bus, _usart3.config->usart_clock);
    stm32_usart_init_device(&_usart3);    
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
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(0xFFF);
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
    IWDG_Enable();
    IWDG_ReloadCounter();
}

void hw_watchdog_reset() {
    /* We're exceedingly careful in this.  We turn on GPIO clocks, read the
     * GPIO to see if the back button is not pressed, read the GPIO clock
     * register to see if we got into trouble, and if and only if we meet
     * all of those, we feed the watchdog.  */
    
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
    
    if (!(GPIOC->IDR & (1 << 3))) /* pressed? */
        goto nofeed;
    if ((GPIOC->MODER & (0x3 << (3 * 2))) != (0x0 << (3 * 2)))
        goto nofeed;
    if ((GPIOC->PUPDR & (0x3 << (3 * 2))) != (0x1 << (3 * 2)))
        goto nofeed;
    
    if (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN)) /* GPIOC clocks? */
        goto nofeed;
    if (RCC->AHB1RSTR & RCC_AHB1RSTR_GPIOCRST) /* in reset? */
        goto nofeed;

    IWDG_ReloadCounter();
    
nofeed:
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
}

/*** ambient light sensor ***/

void hw_ambient_init() {
}

uint16_t hw_ambient_get() {
    return 0;
}

/*** backlight init ***/

void hw_backlight_init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitDef;

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitDef);

    /* And multivac pronounced "and let there be backlight" */
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
}

void hw_backlight_set(uint16_t val) {

}

/* buttons */

#include "stm32_buttons_platform.h"

const stm32_button_t platform_buttons[HW_BUTTON_MAX] = {
    [HW_BUTTON_BACK]   = { GPIO_Pin_3, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource3, RCC_AHB1Periph_GPIOC, EXTI3_IRQn },
    [HW_BUTTON_UP]     = { GPIO_Pin_2, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource2, RCC_AHB1Periph_GPIOA, EXTI2_IRQn },
    [HW_BUTTON_SELECT] = { GPIO_Pin_6, GPIOC, EXTI_PortSourceGPIOC, EXTI_PinSource6, RCC_AHB1Periph_GPIOC, EXTI9_5_IRQn },
    [HW_BUTTON_DOWN]   = { GPIO_Pin_1, GPIOA, EXTI_PortSourceGPIOA, EXTI_PinSource1, RCC_AHB1Periph_GPIOA, EXTI1_IRQn }
};

STM32_BUTTONS_MK_IRQ_HANDLER(3)
STM32_BUTTONS_MK_IRQ_HANDLER(2)
STM32_BUTTONS_MK_IRQ_HANDLER(9_5)
STM32_BUTTONS_MK_IRQ_HANDLER(1)

/* backlight */

#include "stm32_backlight_platform.h"

stm32_backlight_config_t platform_backlight = {
  .pin = GPIO_Pin_5,
  .tim = TIM3,
  .pin_source = GPIO_PinSource5,
  .port = RCC_AHB1Periph_GPIOB,
  .af = GPIO_AF_TIM3,
  .rcc_tim = RCC_APB1Periph_TIM3
};

/* display */

static uint8_t _display_fb[168][20];


void hw_display_init() {
    printf("tintin: hw_display_init\n");

    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);

    GPIO_WriteBit(GPIOB, 1 << 12, 0);
    GPIO_PinAFConfig(GPIOB, 1, GPIO_AF_TIM3);
   
    GPIO_InitTypeDef gpioinit;
    
    gpioinit.GPIO_Pin = (1 << 12);
    gpioinit.GPIO_Mode = GPIO_Mode_OUT;
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
    stm32_spi_init_device(&_spi2);
    
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOC);
}

void hw_display_reset() {
    printf("tintin: hw_display_reset\n");
}

void hw_display_start() {
    printf("tintin: hw_display_start\n");
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOB);
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_SPI2);

    printf("tintin: here we go, slowly blitting %d %d\n", x, y);
    GPIO_WriteBit(GPIOB, 1 << 12, 1);
    delay_us(7);
    stm32_spi_write(&_spi2, 0x80);
    for (int i = 0; i < 168; i++) {
        stm32_spi_write(&_spi2, __RBIT(__REV(168-i)));
        for (int j = 0; j < 18; j++)
            stm32_spi_write(&_spi2, _display_fb[i][17-j]);
        stm32_spi_write(&_spi2, 0);
    }
    stm32_spi_write(&_spi2, 0);
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

#define JEDEC_IDCODE_MICRON_N25Q032A11 0x20BB16 /* bianca / qemu / ev2_5 */
#define JEDEC_IDCODE_MICRON_N25Q064A11 0x20BB17 /* v1_5 */
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

    SPI_I2S_DeInit(SPI1);
    
    spiinit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spiinit.SPI_Mode = SPI_Mode_Master;
    spiinit.SPI_DataSize = SPI_DataSize_8b;
    spiinit.SPI_CPOL = SPI_CPOL_Low;
    spiinit.SPI_CPHA = SPI_CPHA_1Edge;
    spiinit.SPI_NSS = SPI_NSS_Soft;
    spiinit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spiinit.SPI_FirstBit = SPI_FirstBit_MSB;
    spiinit.SPI_CRCPolynomial = 7 /* Um. */;
    SPI_Init(SPI1, &spiinit);
    SPI_Cmd(SPI1, ENABLE);
    
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

    if (part_id != JEDEC_IDCODE_MICRON_N25Q032A11 && part_id != JEDEC_IDCODE_MICRON_N25Q064A11) {
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
    
    /* make sure we are fully clocked out before we drop enable */
//     delay_us(100);
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


void hw_bluetooth_init() {
    rebbleos_module_set_status(MODULE_BLUETOOTH, MODULE_DISABLED, MODULE_ERROR);
}

void bt_device_request_tx() {
}

void HardFault_Handler(uint32_t *sp)
{
    printf("*** HARD FAULT ***\n");
    printf("   R0: %08lx, R1: %08lx, R2: %08lx, R3: %08lx\n", sp[0], sp[1], sp[2], sp[3]);
    printf("  R12: %08lx, LR: %08lx, PC: %08lx, SP: %08lx\n", sp[4], sp[5], sp[6], (uint32_t) sp);
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
