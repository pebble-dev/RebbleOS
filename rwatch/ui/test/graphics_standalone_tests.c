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
    printf("testing Window/Layers\n");

    Window *window;
    window = window_create();
 
    printf("PASS: created window\n");
    
    Layer * layer;
    layer = window_get_root_layer(window);
    
    if (layer == NULL)
    {
        printf("FAIL: Root layer invalid\n");
        exit(1);
    }
    
    printf("PASS: Root layer is good\n");
    
    GRect bounds = {
        .x = 0,
        .y = 0,
        .w = 144,
        .h = 144
    };
    
    layer = layer_create(bounds);
    
    printf("PASS: New Layer\n");
    
    Layer *rlayer = window_get_root_layer(window);
    layer_set_update_proc(rlayer, cb_layer1);
    layer_add_child(rlayer, layer);

    walk_layers(rlayer);
    
    Layer *layer1 = layer_create(bounds);
    Layer *layer2 = layer_create(bounds);
    
    layer_add_child(layer, layer1);
    layer_add_child(layer, layer2);
 
    printf("...\n");
    walk_layers(rlayer);
    
}

void cb_layer1(Layer *layer, GContext *context)
{
    printf("CB layer 1\n");
}
