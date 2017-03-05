#pragma once
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "neographics.h"
#include "layer.h"

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

typedef enum ButtonId
{
    BUTTON_ID_BACK,
    BUTTON_ID_UP,
    BUTTON_ID_SELECT,
    BUTTON_ID_DOWN,
    NUM_BUTTONS
} ButtonId;

uint8_t click_number_of_clicks_counted(ClickRecognizerRef recognizer);
ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer);
bool click_recognizer_is_repeating(ClickRecognizerRef recognizer);

// TODO move me

// TODO in neogfx?
typedef enum GAlign
{
    GAlignCenter,
    GAlignTopLeft,
    GAlignTopRight,
    GAlignTop,
    GAlignLeft,
    GAlignBottom,
    GAlignRight,
    GAlignBottomRight,
    GAlignBottomLeft
} GAlign;


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
