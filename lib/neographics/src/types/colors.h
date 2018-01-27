/*\
|*|
|*|   Neographics: a tiny graphics library.
|*|   Copyright (C) 2016 Johannes Neubrand <johannes_n@icloud.com>
|*|
|*|   This program is free software; you can redistribute it and/or
|*|   modify it under the terms of the GNU General Public License
|*|   as published by the Free Software Foundation; either version 2
|*|   of the License, or (at your option) any later version.
|*|
|*|   This program is distributed in the hope that it will be useful,
|*|   but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|   GNU General Public License for more details.
|*|
|*|   You should have received a copy of the GNU General Public License
|*|   along with this program; if not, write to the Free Software
|*|   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
|*|
\*/

#pragma once
#include "color.h"

#define n_GColorWhiteARGB8                 ((uint8_t)   0b11111111)
#define n_GColorWhite                      ((n_GColor) { .argb = (n_GColorWhiteARGB8) })

#define n_GColorLightGrayARGB8             ((uint8_t)   0b11101010)
#define n_GColorLightGray                  ((n_GColor) { .argb = (n_GColorLightGrayARGB8) })

#define n_GColorDarkGrayARGB8              ((uint8_t)   0b11010101)
#define n_GColorDarkGray                   ((n_GColor) { .argb = (n_GColorDarkGrayARGB8) })

#define n_GColorBlackARGB8                 ((uint8_t)   0b11000000)
#define n_GColorBlack                      ((n_GColor) { .argb = (n_GColorBlackARGB8) })



#define n_GColorOxfordBlueARGB8            ((uint8_t)   0b11000001)
#define n_GColorOxfordBlue                 ((n_GColor) { .argb = (n_GColorOxfordBlueARGB8) })

#define n_GColorDarkBlueARGB8              ((uint8_t)   0b11000010)
#define n_GColorDarkBlue                   ((n_GColor) { .argb = (n_GColorDarkBlueARGB8) })

#define n_GColorBlueARGB8                  ((uint8_t)   0b11000011)
#define n_GColorBlue                       ((n_GColor) { .argb = (n_GColorBlueARGB8) })

#define n_GColorDarkGreenARGB8             ((uint8_t)   0b11000100)
#define n_GColorDarkGreen                  ((n_GColor) { .argb = (n_GColorDarkGreenARGB8) })

#define n_GColorMidnightGreenARGB8         ((uint8_t)   0b11000101)
#define n_GColorMidnightGreen              ((n_GColor) { .argb = (n_GColorMidnightGreenARGB8) })

#define n_GColorCobaltBlueARGB8            ((uint8_t)   0b11000110)
#define n_GColorCobaltBlue                 ((n_GColor) { .argb = (n_GColorCobaltBlueARGB8) })

#define n_GColorBlueMoonARGB8              ((uint8_t)   0b11000111)
#define n_GColorBlueMoon                   ((n_GColor) { .argb = (n_GColorBlueMoonARGB8) })

#define n_GColorIslamicGreenARGB8          ((uint8_t)   0b11001000)
#define n_GColorIslamicGreen               ((n_GColor) { .argb = (n_GColorIslamicGreenARGB8) })

#define n_GColorJaegerGreenARGB8           ((uint8_t)   0b11001001)
#define n_GColorJaegerGreen                ((n_GColor) { .argb = (n_GColorJaegerGreenARGB8) })

#define n_GColorTiffanyBlueARGB8           ((uint8_t)   0b11001010)
#define n_GColorTiffanyBlue                ((n_GColor) { .argb = (n_GColorTiffanyBlueARGB8) })

#define n_GColorVividCeruleanARGB8         ((uint8_t)   0b11001011)
#define n_GColorVividCerulean              ((n_GColor) { .argb = (n_GColorVividCeruleanARGB8) })

#define n_GColorGreenARGB8                 ((uint8_t)   0b11001100)
#define n_GColorGreen                      ((n_GColor) { .argb = (n_GColorGreenARGB8) })

#define n_GColorMalachiteARGB8             ((uint8_t)   0b11001101)
#define n_GColorMalachite                  ((n_GColor) { .argb = (n_GColorMalachiteARGB8) })

#define n_GColorMediumSpringGreenARGB8     ((uint8_t)   0b11001110)
#define n_GColorMediumSpringGreen          ((n_GColor) { .argb = (n_GColorMediumSpringGreenARGB8) })

#define n_GColorCyanARGB8                  ((uint8_t)   0b11001111)
#define n_GColorCyan                       ((n_GColor) { .argb = (n_GColorCyanARGB8) })

#define n_GColorBulgarianRoseARGB8         ((uint8_t)   0b11010000)
#define n_GColorBulgarianRose              ((n_GColor) { .argb = (n_GColorBulgarianRoseARGB8) })

#define n_GColorImperialPurpleARGB8        ((uint8_t)   0b11010001)
#define n_GColorImperialPurple             ((n_GColor) { .argb = (n_GColorImperialPurpleARGB8) })

#define n_GColorIndigoARGB8                ((uint8_t)   0b11010010)
#define n_GColorIndigo                     ((n_GColor) { .argb = (n_GColorIndigoARGB8) })

#define n_GColorElectricUltramarineARGB8   ((uint8_t)   0b11010011)
#define n_GColorElectricUltramarine        ((n_GColor) { .argb = (n_GColorElectricUltramarineARGB8) })

#define n_GColorArmyGreenARGB8             ((uint8_t)   0b11010100)
#define n_GColorArmyGreen                  ((n_GColor) { .argb = (n_GColorArmyGreenARGB8) })

// --- //

#define n_GColorLibertyARGB8               ((uint8_t)   0b11010110)
#define n_GColorLiberty                    ((n_GColor) { .argb = (n_GColorLibertyARGB8) })

#define n_GColorVeryLightBlueARGB8         ((uint8_t)   0b11010111)
#define n_GColorVeryLightBlue              ((n_GColor) { .argb = (n_GColorVeryLightBlueARGB8) })

#define n_GColorKellyGreenARGB8            ((uint8_t)   0b11011000)
#define n_GColorKellyGreen                 ((n_GColor) { .argb = (n_GColorKellyGreenARGB8) })

#define n_GColorMayGreenARGB8              ((uint8_t)   0b11011001)
#define n_GColorMayGreen                   ((n_GColor) { .argb = (n_GColorMayGreenARGB8) })

#define n_GColorCadetBlueARGB8             ((uint8_t)   0b11011010)
#define n_GColorCadetBlue                  ((n_GColor) { .argb = (n_GColorCadetBlueARGB8) })

#define n_GColorPictonBlueARGB8            ((uint8_t)   0b11011011)
#define n_GColorPictonBlue                 ((n_GColor) { .argb = (n_GColorPictonBlueARGB8) })

#define n_GColorBrightGreenARGB8           ((uint8_t)   0b11011100)
#define n_GColorBrightGreen                ((n_GColor) { .argb = (n_GColorBrightGreenARGB8) })

#define n_GColorScreaminGreenARGB8         ((uint8_t)   0b11011101)
#define n_GColorScreaminGreen              ((n_GColor) { .argb = (n_GColorScreaminGreenARGB8) })

#define n_GColorMediumAquamarineARGB8      ((uint8_t)   0b11011110)
#define n_GColorMediumAquamarine           ((n_GColor) { .argb = (n_GColorMediumAquamarineARGB8) })

#define n_GColorElectricBlueARGB8          ((uint8_t)   0b11011111)
#define n_GColorElectricBlue               ((n_GColor) { .argb = (n_GColorElectricBlueARGB8) })

#define n_GColorDarkCandyAppleRedARGB8     ((uint8_t)   0b11100000)
#define n_GColorDarkCandyAppleRed          ((n_GColor) { .argb = (n_GColorDarkCandyAppleRedARGB8) })

#define n_GColorJazzberryJamARGB8          ((uint8_t)   0b11100001)
#define n_GColorJazzberryJam               ((n_GColor) { .argb = (n_GColorJazzberryJamARGB8) })

#define n_GColorPurpleARGB8                ((uint8_t)   0b11100010)
#define n_GColorPurple                     ((n_GColor) { .argb = (n_GColorPurpleARGB8) })

#define n_GColorVividVioletARGB8           ((uint8_t)   0b11100011)
#define n_GColorVividViolet                ((n_GColor) { .argb = (n_GColorVividVioletARGB8) })

#define n_GColorWindsorTanARGB8            ((uint8_t)   0b11100100)
#define n_GColorWindsorTan                 ((n_GColor) { .argb = (n_GColorWindsorTanARGB8) })

#define n_GColorRoseValeARGB8              ((uint8_t)   0b11100101)
#define n_GColorRoseVale                   ((n_GColor) { .argb = (n_GColorRoseValeARGB8) })

#define n_GColorPurpureusARGB8             ((uint8_t)   0b11100110)
#define n_GColorPurpureus                  ((n_GColor) { .argb = (n_GColorPurpureusARGB8) })

#define n_GColorLavenderIndigoARGB8        ((uint8_t)   0b11100111)
#define n_GColorLavenderIndigo             ((n_GColor) { .argb = (n_GColorLavenderIndigoARGB8) })

#define n_GColorLimerickARGB8              ((uint8_t)   0b11101000)
#define n_GColorLimerick                   ((n_GColor) { .argb = (n_GColorLimerickARGB8) })

#define n_GColorBrassARGB8                 ((uint8_t)   0b11101001)
#define n_GColorBrass                      ((n_GColor) { .argb = (n_GColorBrassARGB8) })

// --- //

#define n_GColorBabyBlueEyesARGB8          ((uint8_t)   0b11101011)
#define n_GColorBabyBlueEyes               ((n_GColor) { .argb = (n_GColorBabyBlueEyesARGB8) })

#define n_GColorSpringBudARGB8             ((uint8_t)   0b11101100)
#define n_GColorSpringBud                  ((n_GColor) { .argb = (n_GColorSpringBudARGB8) })

#define n_GColorInchwormARGB8              ((uint8_t)   0b11101101)
#define n_GColorInchworm                   ((n_GColor) { .argb = (n_GColorInchwormARGB8) })

#define n_GColorMintGreenARGB8             ((uint8_t)   0b11101110)
#define n_GColorMintGreen                  ((n_GColor) { .argb = (n_GColorMintGreenARGB8) })

#define n_GColorCelesteARGB8               ((uint8_t)   0b11101111)
#define n_GColorCeleste                    ((n_GColor) { .argb = (n_GColorCelesteARGB8) })

#define n_GColorRedARGB8                   ((uint8_t)   0b11110000)
#define n_GColorRed                        ((n_GColor) { .argb = (n_GColorRedARGB8) })

#define n_GColorFollyARGB8                 ((uint8_t)   0b11110001)
#define n_GColorFolly                      ((n_GColor) { .argb = (n_GColorFollyARGB8) })

#define n_GColorFashionMagentaARGB8        ((uint8_t)   0b11110010)
#define n_GColorFashionMagenta             ((n_GColor) { .argb = (n_GColorFashionMagentaARGB8) })

#define n_GColorMagentaARGB8               ((uint8_t)   0b11110011)
#define n_GColorMagenta                    ((n_GColor) { .argb = (n_GColorMagentaARGB8) })

#define n_GColorOrangeARGB8                ((uint8_t)   0b11110100)
#define n_GColorOrange                     ((n_GColor) { .argb = (n_GColorOrangeARGB8) })

#define n_GColorSunsetOrangeARGB8          ((uint8_t)   0b11110101)
#define n_GColorSunsetOrange               ((n_GColor) { .argb = (n_GColorSunsetOrangeARGB8) })

#define n_GColorBrilliantRoseARGB8         ((uint8_t)   0b11110110)
#define n_GColorBrilliantRose              ((n_GColor) { .argb = (n_GColorBrilliantRoseARGB8) })

#define n_GColorShockingPinkARGB8          ((uint8_t)   0b11110111)
#define n_GColorShockingPink               ((n_GColor) { .argb = (n_GColorShockingPinkARGB8) })

#define n_GColorChromeYellowARGB8          ((uint8_t)   0b11111000)
#define n_GColorChromeYellow               ((n_GColor) { .argb = (n_GColorChromeYellowARGB8) })

#define n_GColorRajahARGB8                 ((uint8_t)   0b11111001)
#define n_GColorRajah                      ((n_GColor) { .argb = (n_GColorRajahARGB8) })

#define n_GColorMelonARGB8                 ((uint8_t)   0b11111010)
#define n_GColorMelon                      ((n_GColor) { .argb = (n_GColorMelonARGB8) })

#define n_GColorRichBrilliantLavenderARGB8 ((uint8_t)   0b11111011)
#define n_GColorRichBrilliantLavender      ((n_GColor) { .argb = (n_GColorRichBrilliantLavenderARGB8) })

#define n_GColorYellowARGB8                ((uint8_t)   0b11111100)
#define n_GColorYellow                     ((n_GColor) { .argb = (n_GColorYellowARGB8) })

#define n_GColorIcterineARGB8              ((uint8_t)   0b11111101)
#define n_GColorIcterine                   ((n_GColor) { .argb = (n_GColorIcterineARGB8) })

#define n_GColorPastelYellowARGB8          ((uint8_t)   0b11111110)
#define n_GColorPastelYellow               ((n_GColor) { .argb = (n_GColorPastelYellowARGB8) })

// #define n_GColorWhiteARGB8                 ((uint8_t)   0b11111111)
// #define n_GColorWhite                      ((n_GColor) { .argb = (n_GColorWhiteARGB8) })


#define n_GColorClearARGB8                 ((uint8_t)   0b00000000)
#define n_GColorClear                      ((n_GColor) { .argb = (n_GColorClearARGB8) })
#define GColorClear n_GColorClear 
