/* graphics_standalone_tests.c
 * routines for [...]
 * RebbleOS core
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "graphics_common.h"

void test_window_layer(void);

void cb_layer1(Layer *layer, GContext *context);

void main(void)
{
    test_window_layer();
}

void test_window_layer(void)
{
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "testing Window/Layers");

    Window *window;
    window = window_create();
 
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "PASS: created window");
    
    Layer * layer;
    layer = window_get_root_layer(window);
    
    if (layer == NULL)
    {
		SYS_LOG("graphics_test", APP_LOG_LEVEL_ERROR, "FAIL: Root layer invalid");
        exit(1);
    }
    
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "PASS: Root layer is good");
    
    GRect bounds = {
        .x = 0,
        .y = 0,
        .w = 144,
        .h = 144
    };
    
    layer = layer_create(bounds);
    
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "PASS: New Layer");
    
    Layer *rlayer = window_get_root_layer(window);
    layer_set_update_proc(rlayer, cb_layer1);
    layer_add_child(rlayer, layer);

    walk_layers(rlayer);
    
    Layer *layer1 = layer_create(bounds);
    Layer *layer2 = layer_create(bounds);
    
    layer_add_child(layer, layer1);
    layer_add_child(layer, layer2);
 
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "...");
    walk_layers(rlayer);
    
}

void cb_layer1(Layer *layer, GContext *context)
{
	SYS_LOG("graphics_test", APP_LOG_LEVEL_INFO, "CB layer 1");
}
