#pragma once
/* click_config.h
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "neographics.h"
#include "layer.h"
#include "platform.h"
#include "pebble_defines.h"
#include "gbitmap.h"

/* XXX: move this to buttons.h so we don't have to depend on platform.h (ick) here */
typedef enum ButtonId
{
    BUTTON_ID_BACK   = HW_BUTTON_BACK,
    BUTTON_ID_UP     = HW_BUTTON_UP,
    BUTTON_ID_SELECT = HW_BUTTON_SELECT,
    BUTTON_ID_DOWN   = HW_BUTTON_DOWN,
    NUM_BUTTONS      = HW_BUTTON_MAX
} ButtonId;


struct Layer;
struct GRect;

typedef void *ClickRecognizerRef;
typedef void (*ClickHandler)(ClickRecognizerRef recognizer, void *context);

typedef struct ClickConfig
{
    void *context;
    struct 
    {
        ClickHandler handler;
        uint16_t repeat_interval_ms;
    } click;
    struct 
    {
        uint8_t min;
        uint8_t max;
        bool last_click_only;
        ClickHandler handler;
        uint16_t timeout;
    } multi_click;
    struct 
    {
        uint16_t delay_ms;
        ClickHandler handler;
        ClickHandler release_handler;
    } long_click;
    struct 
    {
        ClickHandler up_handler;
        ClickHandler down_handler;
        void *context;
    } raw;
} ClickConfig;

typedef void (*ClickConfigProvider)(void *context);

uint8_t click_number_of_clicks_counted(ClickRecognizerRef recognizer);
ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer);
bool click_recognizer_is_repeating(ClickRecognizerRef recognizer);

// TODO move me


typedef struct GColourPair
{
    GColor foreground;
    GColor background;
} GColourPair;

typedef struct ContentIndicatorConfig
{
    Layer *layer;
    bool times_out;
    GAlign alignment;
    GColourPair colors;
} ContentIndicatorConfig;

typedef struct ContentIndicator ContentIndicator;

typedef enum ContentIndicatorDirection 
{
    ContentIndicatorDirectionUp,
    ContentIndicatorDirectionDown,
    NumContentIndicatorDirections
} ContentIndicatorDirection;
