#pragma once
/* inverter_layer.c
 *
 * InverterLayer component.
 * 
 * Author: Carter <barry.carter@gmail.com>
 */

#include "librebble.h"

typedef struct InverterLayer
{
    Layer layer;
} InverterLayer;

void inverter_layer_ctor(InverterLayer *ilayer, GRect frame);
void inverter_layer_dtor(InverterLayer *ilayer);
InverterLayer *inverter_layer_create(GRect frame);
void inverter_layer_destroy(InverterLayer *ilayer);
Layer *inverter_layer_get_layer(InverterLayer *ilayer);