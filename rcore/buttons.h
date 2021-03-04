#pragma once
/* buttons.h
 * routines for Debouncing and sending buttons through a click, multi, long press handler
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include "rebbleos.h"
#include "librebble.h"

#define butDEBOUNCE_DELAY       ( pdMS_TO_TICKS(2) )

#define BUTTON_STATE_PRESSED    0
#define BUTTON_STATE_RELEASED   1
#define BUTTON_STATE_LONG       2
#define BUTTON_STATE_REPEATING  3
#define BUTTON_STATE_MULTI      4
#define BUTTON_STATE_MULTI_DONE 5

uint8_t rcore_buttons_init(void);

void button_single_click_subscribe(ButtonId button_id, ClickHandler handler);
void button_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);
void button_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);
void button_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);
void button_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context);
void button_unsubscribe_all(void);

/**
 * Click configs
 */
uint8_t click_number_of_clicks_counted(ClickRecognizerRef recognizer);
ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer);
bool click_recognizer_is_repeating(ClickRecognizerRef recognizer);

void button_set_click_context(ButtonId button_id, void *context);

/**
 * Check if a button is subscribed to any short click click handlers
 */
uint8_t button_short_click_is_subscribed(ButtonId button_id);
