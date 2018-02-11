#pragma once
#include "platform_config.h"
/* pebble-defines.h
 * Pebble defined header stuff
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#ifdef PBL_RECT
#define PBL_IF_RECT_ELSE(rct, round) (rct)
#else
#define PBL_IF_RECT_ELSE(rct, round) (round)
#endif

#if defined REBBLE_PLATFORM_TINTIN
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (tintin)
#elif defined REBBLE_PLATFORM_SNOWY
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (snowy)
#elif defined REBBLE_PLATFORM_CHALK
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (chalk)
#else
#error Add the new platform to PBL_PLATFORM_SWITCH in pebble_defines.h
#endif

#define graphics_context_set_fill_color n_graphics_context_set_fill_color
#define graphics_context_set_stroke_color n_graphics_context_set_stroke_color
#define graphics_context_set_stroke_width n_graphics_context_set_stroke_width
#define graphics_context_set_antialiased n_graphics_context_set_antialiased

#define GColorFromRGBA n_GColorFromRGBA
#define GColorFromRGB n_GColorFromRGB
#define GColor8 n_GColor8


// text redefines
#define GTextOverflowMode n_GTextOverflowMode
#define GTextOverflowModeTrailingEllipsis n_GTextOverflowModeTrailingEllipsis
#define GFont n_GFont
#define GTextAlignment n_GTextAlignment
#define GTextAlignmentLeft n_GTextAlignmentLeft
#define GTextAlignmentCenter n_GTextAlignmentCenter
#define GTextAlignmentRight n_GTextAlignmentRight
#define GTextAttributes n_GTextAttributes


// math
#define TRIG_MAX_RATIO 0xffff
#define TRIG_MAX_ANGLE 0x10000

#define GContext n_GContext


#define GSize n_GSize
#define GColor n_GColor
#define GRect n_GRect
#define GPoint n_GPoint

#define GColorOxfordBlue n_GColorOxfordBlue
#define GColorDarkBlue n_GColorDarkBlue
#define GColorBlue n_GColorBlue
#define GColorDarkGreen n_GColorDarkGreen
#define GColorMidnightGreen n_GColorMidnightGreen
#define GColorCobaltBlue n_GColorCobaltBlue
#define GColorBlueMoon n_GColorBlueMoon
#define GColorIslamicGreen n_GColorIslamicGreen
#define GColorJaegerGreen n_GColorJaegerGreen
#define GColorTiffanyBlue n_GColorTiffanyBlue
#define GColorVividCerulean n_GColorVividCerulean
#define GColorGreen n_GColorGreen
#define GColorMalachite n_GColorMalachite
#define GColorMediumSpringGreen n_GColorMediumSpringGreen
#define GColorCyan n_GColorCyan
#define GColorBulgarianRose n_GColorBulgarianRose
#define GColorImperialPurple n_GColorImperialPurple
#define GColorIndigo n_GColorIndigo
#define GColorElectricUltramarine n_GColorElectricUltramarine
#define GColorArmyGreen n_GColorArmyGreen
#define GColorDarkGray n_GColorDarkGray
#define GColorLiberty n_GColorLiberty
#define GColorVeryLightBlue n_GColorVeryLightBlue
#define GColorKellyGreen n_GColorKellyGreen
#define GColorMayGreen n_GColorMayGreen
#define GColorCadetBlue n_GColorCadetBlue
#define GColorPictonBlue n_GColorPictonBlue
#define GColorBrightGreen n_GColorBrightGreen
#define GColorScreaminGreen n_GColorScreaminGreen
#define GColorMediumAquamarine n_GColorMediumAquamarine
#define GColorElectricBlue n_GColorElectricBlue
#define GColorDarkCandyAppleRed n_GColorDarkCandyAppleRed
#define GColorJazzberryJam n_GColorJazzberryJam
#define GColorPurple n_GColorPurple
#define GColorVividViolet n_GColorVividViolet
#define GColorWindsorTan n_GColorWindsorTan
#define GColorRoseVale n_GColorRoseVale
#define GColorPurpureus n_GColorPurpureus
#define GColorLavenderIndigo n_GColorLavenderIndigo
#define GColorLimerick n_GColorLimerick
#define GColorBrass n_GColorBrass
#define GColorLightGray n_GColorLightGray 
#define GColorBabyBlueEyes n_GColorBabyBlueEyes
#define GColorSpringBud n_GColorSpringBud
#define GColorInchworm n_GColorInchworm
#define GColorMintGreen n_GColorMintGreen
#define GColorCeleste n_GColorCeleste
#define GColorRed n_GColorRed 
#define GColorFolly n_GColorFolly
#define GColorFashionMagenta n_GColorFashionMagenta
#define GColorMagenta n_GColorMagenta
#define GColorOrange n_GColorOrange 
#define GColorSunsetOrange n_GColorSunsetOrange
#define GColorBrilliantRose n_GColorBrilliantRose
#define GColorShockingPink n_GColorShockingPink
#define GColorChromeYellow n_GColorChromeYellow
#define GColorRajah n_GColorRajah
#define GColorMelon n_GColorMelon
#define GColorRichBrilliantLavender n_GColorRichBrilliantLavender
#define GColorYellow n_GColorYellow
#define GColorIcterine n_GColorIcterine
#define GColorPastelYellow n_GColorPastelYellow
#define GColorWhite n_GColorWhite 
#define GColorBlack n_GColorBlack

#define GColorClearARGB8 ((uint8_t)0b00000000)
#define GColorClear ((GColor8){.argb=GColorClearARGB8})


#define GCornerNone n_GCornerNone

#define graphics_context_set_text_color n_graphics_context_set_text_color
#include "gbitmap.h"
