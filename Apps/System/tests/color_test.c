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

static n_GColor colors[] = {
    GColorOxfordBlue,
    GColorDarkBlue,
    GColorBlue,
    GColorDarkGreen,
    GColorMidnightGreen,
    GColorCobaltBlue,
    GColorBlueMoon,
    GColorIslamicGreen,//
    GColorJaegerGreen,
    GColorTiffanyBlue,
    GColorVividCerulean,
    GColorGreen,//
    GColorMalachite,
    GColorMediumSpringGreen,
    GColorCyan,
    GColorBulgarianRose,////
    GColorImperialPurple,//second row
    GColorIndigo,
    GColorElectricUltramarine,
    GColorArmyGreen, // se 4
    GColorDarkGray,
    GColorLiberty,//
    GColorVeryLightBlue,
    GColorKellyGreen,//
    GColorMayGreen,//
    GColorCadetBlue,//
    GColorPictonBlue,
    GColorBrightGreen,//
    GColorScreaminGreen,//
    GColorMediumAquamarine,
    GColorElectricBlue,//
    GColorDarkCandyAppleRed,//
    GColorJazzberryJam,//third row
    GColorPurple,//
    GColorVividViolet,//
    GColorWindsorTan, //
    GColorRoseVale,//
    GColorPurpureus,//
    GColorLavenderIndigo,//
    GColorLimerick,//
    GColorBrass,//
    GColorLightGray,//
    GColorBabyBlueEyes,//
    GColorSpringBud,//
    GColorInchworm,//
    GColorMintGreen,//
    GColorCeleste,//
    GColorRed,//
    GColorFolly, // forth row
    GColorFashionMagenta,//
    GColorMagenta,//
    GColorOrange,//
    GColorSunsetOrange,//
    GColorBrilliantRose,//
    GColorShockingPink,//
    GColorChromeYellow,//
    GColorRajah,//
    GColorMelon,//
    GColorRichBrilliantLavender,//
    GColorYellow,//
    GColorIcterine,//
    GColorPastelYellow,//
    GColorWhite,//
    GColorBlack//
};

bool color_test_init(Window *window)
{
    SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "Init: Color Test");
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
    SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "Exec: Color Test");
    return true;
}

bool color_test_deinit(void)
{
    SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "De-Init: Color Test");
    layer_remove_from_parent(_test_layer);
    if (_test_layer != NULL)
        layer_destroy(_test_layer);
    _test_layer = NULL;
    return true;
}

static void _color_test_layer_update_proc(Layer *layer, GContext *ctx) 
{
    SYS_LOG("test", APP_LOG_LEVEL_DEBUG, "DRAW");
    // 9 x 42
    // 16 * 4
    int i = 0;
    int y = 0;
    int x = 0;
    for (y = 0; y < 4; y++) {
        for(x = 0; x < 16; x++) {
            graphics_context_set_fill_color(ctx, colors[i]);
            graphics_fill_rect(ctx, GRect(x*9, y * 42, 9, 42), 0, GCornerNone);
            i++;
        }
    }
}
