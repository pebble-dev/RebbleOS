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
#define PBL_IF_ROUND_ELSE(round, rct) (rct)
#else
#define PBL_IF_RECT_ELSE(rct, round) (round)
#define PBL_IF_ROUND_ELSE(round, rct) (round)
#endif

#ifdef PBL_BW
#define PBL_IF_COLOR_ELSE(color, bw) (bw)
#define PBL_IF_BW_ELSE(bw, color) (bw)
#else
#define PBL_IF_COLOR_ELSE(color, bw) (color)
#define PBL_IF_BW_ELSE(bw, color) (color)
#endif

#if defined REBBLE_PLATFORM_TINTIN
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (tintin)
#define PBL_IF_MICROPHONE_ELSE(mic, nomic) (nomic)
#define PBL_IF_SMARTSTRAP_ELSE(smart, nosmart) (nosmart)
#define PBL_IF_HEALTH_ELSE(health, nohealth) (nohealth)
#elif defined REBBLE_PLATFORM_SNOWY
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (snowy)
#define PBL_IF_MICROPHONE_ELSE(mic, nomic) (mic)
#define PBL_IF_SMARTSTRAP_ELSE(smart, nosmart) (smart)
#define PBL_IF_HEALTH_ELSE(health, nohealth) (health)
#elif defined REBBLE_PLATFORM_CHALK
#define PBL_PLATFORM_SWITCH(tintin, snowy, chalk, diorite, emery) (chalk)
#define PBL_IF_MICROPHONE_ELSE(mic, nomic) (mic)
#define PBL_IF_SMARTSTRAP_ELSE(smart, nosmart) (smart)
#define PBL_IF_HEALTH_ELSE(health, nohealth) (health)
#else
#error Add the new platform to PBL_PLATFORM_SWITCH in pebble_defines.h
#endif

#define GCompOp n_GCompOp
#define GCompOpAssign n_GCompOpAssign
#define GCompOpAssignInverted n_GCompOpAssignInverted
#define GCompOpOr n_GCompOpOr
#define GCompOpAnd n_GCompOpAnd
#define GCompOpClear n_GCompOpClear
#define GCompOpSet n_GCompOpSet

#define GContext n_GContext
#define graphics_context_set_fill_color n_graphics_context_set_fill_color
#define graphics_context_set_text_color n_graphics_context_set_text_color
#define graphics_context_set_stroke_color n_graphics_context_set_stroke_color
#define graphics_context_set_stroke_width n_graphics_context_set_stroke_width
#define graphics_context_set_antialiased n_graphics_context_set_antialiased
#define graphics_context_set_compositing_mode n_graphics_context_set_compositing_mode

#define GColorFromRGBA n_GColorFromRGBA
#define GColorFromRGB n_GColorFromRGB
#define GColor8 n_GColor8

// gbitmap
#define GBitmapFormat enum n_GBitmapFormat
#define GBitmapFormat1Bit n_GBitmapFormat1Bit
#define GBitmapFormat8Bit n_GBitmapFormat8Bit
#define GBitmapFormat1BitPalette n_GBitmapFormat1BitPalette
#define GBitmapFormat2BitPalette n_GBitmapFormat2BitPalette
#define GBitmapFormat4BitPalette n_GBitmapFormat4BitPalette
#define GBitmapFormat8BitCircular n_GBitmapFormat8BitCircular

#define GBitmap struct n_GBitmap
#define gbitmap_destroy n_gbitmap_destroy
#define gbitmap_get_bytes_per_row n_gbitmap_get_bytes_per_row
#define gbitmap_get_format n_gbitmap_get_format
#define gbitmap_get_data n_gbitmap_get_data
#define gbitmap_set_data n_gbitmap_set_data
#define gbitmap_get_bounds n_gbitmap_get_bounds
#define gbitmap_set_bounds n_gbitmap_set_bounds
#define gbitmap_get_palette n_gbitmap_get_palette
#define gbitmap_set_palette n_gbitmap_set_palette
#define gbitmap_create_as_sub_bitmap n_gbitmap_create_as_sub_bitmap
#define gbitmap_create_blank n_gbitmap_create_blank
#define gbitmap_create_blank_with_palette n_gbitmap_create_blank_with_palette
#define gbitmap_create_palettized_from_1bit n_gbitmap_create_palettized_from_1bit

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


#define GSize n_GSize
#define GColor n_GColor
#define GRect n_GRect
#define GPoint n_GPoint
#define gsize_equal n_gsize_equal
#define gcolor_equal n_gcolor_equal
#define gcolor_legible_over n_gcolor_legible_over
#define gpoint_equal n_gpoint_equal
#define grect_center_point n_grect_center_point
#define grect_equal n_grect_equal
#define grect_is_empty n_grect_is_empty
#define grect_clip n_grect_clip
#define grect_contains_point n_grect_contains_point
#define grect_crop n_grect_crop

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
#define GColorClear n_GColorClear


#define GCornerNone n_GCornerNone

#include "gbitmap.h"
