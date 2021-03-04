#include "rebbleos.h"
#include "systemapp.h"
#include "menu.h"
#include "status_bar_layer.h"
#include "test_defs.h"

static Layer *s_test_layer;
static TextLayer *s_multiline_label_layer;
static TextLayer *s_singleline_label_layer;
static GTextAlignment s_text_alignment = GTextAlignmentLeft;
static int8_t s_string_index = 0;

static char *s_test_strings[] =
{
    /* Two lines                                */  "Press Select to Change Alignment",
    /* One line                                 */  "Hello, World!",
    /* Three lines                              */  "The quick brown fox jumped over the lazy dog.",
    /* Really long sentence                     */  "Buffalo buffalo Buffalo bufallo buffalo buffalo Buffalo buffalo.",
    /* Single codepoint lengths w/o breakables  */  "Pneumonoultramicroscopicsilicovolcanoconiosis",
    /* User-defined breaks                      */  "A world of dew,\nAnd within every dewdrop\nA world of struggle.",
    /* Mixed codepoint lengths w/ postbreakables*/ "I want to visit 東京, Αθήνα, मुंबई, القاهرة, and 서울시. 😊",
    /* Mixed codepoint lengths w/o breakables   */ "😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊",
};

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    s_text_alignment = s_text_alignment == GTextAlignmentLeft ? GTextAlignmentCenter : (s_text_alignment == GTextAlignmentCenter ? GTextAlignmentRight : GTextAlignmentLeft);

    text_layer_set_text_alignment(s_multiline_label_layer, s_text_alignment);

    layer_mark_dirty(window_get_root_layer(s_test_layer));
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    s_string_index--;

    if (s_string_index < 0)
        s_string_index = (int)(sizeof(s_test_strings) / sizeof(s_test_strings[0])) - 1;

    text_layer_set_text(s_multiline_label_layer, s_test_strings[s_string_index]);
    layer_mark_dirty(window_get_root_layer(s_test_layer));
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    s_string_index = (s_string_index + 1) % (int)(sizeof(s_test_strings) / sizeof(s_test_strings[0]));
    text_layer_set_text(s_multiline_label_layer, s_test_strings[s_string_index]);
    layer_mark_dirty(window_get_root_layer(s_test_layer));
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);

}

bool text_alignment_test_init(Window *window)
{
    s_test_layer = window_get_root_layer(window);
    GRect rect = layer_get_bounds(s_test_layer);

    s_multiline_label_layer = text_layer_create(rect);
    text_layer_set_text(s_multiline_label_layer, s_test_strings[s_string_index]);
    text_layer_set_text_alignment(s_multiline_label_layer, s_text_alignment);
    text_layer_set_font(s_multiline_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(s_test_layer, text_layer_get_layer(s_multiline_label_layer));

    rect.origin.y = rect.size.h - (4 * n_graphics_font_get_line_height(fonts_get_system_font(FONT_KEY_GOTHIC_14)));

    s_singleline_label_layer = text_layer_create(rect);
    text_layer_set_text(s_singleline_label_layer, "Up/Down: Change Strings\nSelect: Change Alignment");
    text_layer_set_text_alignment(s_singleline_label_layer, GTextAlignmentCenter);
    text_layer_set_font(s_singleline_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(s_test_layer, text_layer_get_layer(s_singleline_label_layer));

    window_set_click_config_provider(window, click_config_provider);

    return true;
}

bool text_alignment_test_exec(void)
{
    return true;
}

bool text_alignment_test_deinit(void)
{
    text_layer_destroy(s_multiline_label_layer);
    text_layer_destroy(s_singleline_label_layer);

    layer_destroy(s_test_layer);

    test_complete(true);
    return true;
}
