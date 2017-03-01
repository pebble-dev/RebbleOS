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
#include <stm32f2xx_rcc.h>
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

void hw_buttons_init() {
}

uint8_t hw_button_pressed(button_t *button) {
	return 0;
}

buttons_t buttons;

/* display */

void hw_display_init() {
}

void hw_display_reset() {
}

void hw_display_start() {
}

void hw_display_start_frame(uint8_t x, uint8_t y) {
}

#include "display.h"

display_t display;

void scanline_rgb888pixel_to_frambuffer(UG_S16 x, UG_S16 y, UG_COLOR c) {
}

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

/* vibrate */

void hw_vibrate_init() {
}

void hw_vibrate_enable(uint8_t enabled) {
}
