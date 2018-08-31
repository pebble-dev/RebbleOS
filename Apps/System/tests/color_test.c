/* color_test.c
 * routines for showing a palette
 * libRebbleOS
 *
 * Author: Author: Chris Multhaupt <chris.multhaupt@gmail.com>.
 */
#include "rebbleos.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"

static Window *_main_window;
static Layer *_test_layer;
static void _color_test_layer_update_proc(Layer *layer, GContext *ctx);

static const uint8_t colors[] = {
    n_GColorOxfordBlueARGB8,
    n_GColorDarkBlueARGB8,
    n_GColorBlueARGB8,
    n_GColorDarkGreenARGB8,
    n_GColorMidnightGreenARGB8,
    n_GColorCobaltBlueARGB8,
    n_GColorBlueMoonARGB8,
    n_GColorIslamicGreenARGB8,//
    n_GColorJaegerGreenARGB8,
    n_GColorTiffanyBlueARGB8,
    n_GColorVividCeruleanARGB8,
    n_GColorGreenARGB8,//
    n_GColorMalachiteARGB8,
    n_GColorMediumSpringGreenARGB8,
    n_GColorCyanARGB8,
    n_GColorBulgarianRoseARGB8,////
    n_GColorImperialPurpleARGB8,//second row
    n_GColorIndigoARGB8,
    n_GColorElectricUltramarineARGB8,
    n_GColorArmyGreenARGB8, // se 4
    n_GColorDarkGrayARGB8,
    n_GColorLibertyARGB8,//
    n_GColorVeryLightBlueARGB8,
    n_GColorKellyGreenARGB8,//
    n_GColorMayGreenARGB8,//
    n_GColorCadetBlueARGB8,//
    n_GColorPictonBlueARGB8,
    n_GColorBrightGreenARGB8,//
    n_GColorScreaminGreenARGB8,//
    n_GColorMediumAquamarineARGB8,
    n_GColorElectricBlueARGB8,//
    n_GColorDarkCandyAppleRedARGB8,//
    n_GColorJazzberryJamARGB8,//third row
    n_GColorPurpleARGB8,//
    n_GColorVividVioletARGB8,//
    n_GColorWindsorTanARGB8, //
    n_GColorRoseValeARGB8,//
    n_GColorPurpureusARGB8,//
    n_GColorLavenderIndigoARGB8,//
    n_GColorLimerickARGB8,//
    n_GColorBrassARGB8,//
    n_GColorLightGrayARGB8,//
    n_GColorBabyBlueEyesARGB8,//
    n_GColorSpringBudARGB8,//
    n_GColorInchwormARGB8,//
    n_GColorMintGreenARGB8,//
    n_GColorCelesteARGB8,//
    n_GColorRedARGB8,//
    n_GColorFollyARGB8, // forth row
    n_GColorFashionMagentaARGB8,//
    n_GColorMagentaARGB8,//
    n_GColorOrangeARGB8,//
    n_GColorSunsetOrangeARGB8,//
    n_GColorBrilliantRoseARGB8,//
    n_GColorShockingPinkARGB8,//
    n_GColorChromeYellowARGB8,//
    n_GColorRajahARGB8,//
    n_GColorMelonARGB8,//
    n_GColorRichBrilliantLavenderARGB8,//
    n_GColorYellowARGB8,//
    n_GColorIcterineARGB8,//
    n_GColorPastelYellowARGB8,//
    n_GColorWhiteARGB8,//
    n_GColorBlackARGB8//
};

bool color_test_init(Window *window)
{
    APP_LOG("test", APP_LOG_LEVEL_DEBUG, "Init: Color Test");
    _main_window = window;
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    _test_layer = layer_create(bounds);
    layer_add_child(window_layer, _test_layer);
    
    layer_set_update_proc(_test_layer, _color_test_layer_update_proc);
    layer_mark_dirty(_test_layer);
    return true;
}

bool color_test_exec(void)
{
    APP_LOG("test", APP_LOG_LEVEL_DEBUG, "Exec: Color Test");
    return true;
}

bool color_test_deinit(void)
{
    APP_LOG("test", APP_LOG_LEVEL_DEBUG, "De-Init: Color Test");
    layer_remove_from_parent(_test_layer);
    if (_test_layer != NULL)
        layer_destroy(_test_layer);
    _test_layer = NULL;
    return true;
}

static void _color_test_layer_update_proc(Layer *layer, GContext *ctx) 
{
    APP_LOG("test", APP_LOG_LEVEL_DEBUG, "DRAW");
    // 9 x 42
    // 16 * 4
    int i = 0;
    int y = 0;
    int x = 0;
    for (y = 0; y < 4; y++) {
        for(x = 0; x < 16; x++) {
            graphics_context_set_fill_color(ctx, (GColor)colors[i]);
            graphics_fill_rect(ctx, GRect(x*9, y * 42, 9, 42), 0, GCornerNone);
            i++;
        }
    }
}
