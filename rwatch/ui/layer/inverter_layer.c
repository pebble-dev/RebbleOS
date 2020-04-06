/* inverter_layer.c
 *
 * InverterLayer component.
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "librebble.h"
#include "utils.h"
#include "graphics_wrapper.h"
#include "inverter_layer.h"

static void _inverter_layer_update_proc(Layer *layer, GContext *nGContext);

void inverter_layer_ctor(InverterLayer *ilayer, GRect frame)
{
    layer_ctor(&ilayer->layer, frame);
    layer_set_update_proc(&ilayer->layer, _inverter_layer_update_proc);
}

void inverter_layer_dtor(InverterLayer *ilayer)
{
    layer_dtor(&ilayer->layer);
}

InverterLayer *inverter_layer_create(GRect frame)
{
    InverterLayer *mlayer = (InverterLayer *)app_calloc(1, sizeof(InverterLayer));
    inverter_layer_ctor(mlayer, frame);
    return mlayer;
}

void inverter_layer_destroy(InverterLayer *ilayer)
{    
    inverter_layer_dtor(ilayer);
    app_free(ilayer);
}

Layer *inverter_layer_get_layer(InverterLayer *ilayer)
{
    return (Layer *)ilayer;
}


n_GColor n_graphics_get_pixel(GContext * ctx, GPoint point)
{
#ifdef PBL_BW
    uint32_t byteIndex = point.y * DISPLAY_COLS + point.x / 8;
    uint32_t bitMask = 1 << (point.x % 8);
    return (ctx->fbuf[byteIndex] & bitMask) ? n_GColorWhite : n_GColorBlack;
#else
    uint32_t index = point.y * DISPLAY_COLS + point.x;
    return (GColor) { .argb = ctx->fbuf[index] | 0b11000000 };
#endif
}

static void _inverter_layer_update_proc(Layer *layer, GContext *nGContext)
{
    app_running_thread *t = appmanager_get_current_thread();
    for(int y = 0; y < layer->bounds.size.h; y++) {
        for(int x = 0; x < layer->bounds.size.w; x++) {
            GPoint p = GPoint(x + layer->bounds.origin.x, y + layer->bounds.origin.y);
            n_graphics_set_pixel(
                t->graphics_context, 
                p, 
                (GColor) { .argb =(~(n_graphics_get_pixel(t->graphics_context, p)).argb) });
        }
    }
}