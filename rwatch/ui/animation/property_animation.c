/* property_animation.c
 * a convenience animation for 
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "appmanager.h"
#include "FreeRTOS.h"
#include "property_animation.h"
#include "animation.h"

void property_animation_update_grect(PropertyAnimation * property_animation, const uint32_t distance_normalized)
{
    if (property_animation->impl.accessors.getter.grect != NULL && property_animation->impl.accessors.setter.grect != NULL)
    {
        GRect *from = (GRect *) property_animation->values.from;
        GRect *to = (GRect *) property_animation->values.to;
        
        GRect new_rect = GRect(ANIM_LERP(from->origin.x, 
                               to->origin.x, 
                               distance_normalized), 
                               ANIM_LERP(from->origin.y, to->origin.y, distance_normalized), 
                               ANIM_LERP(from->size.w, to->size.w, distance_normalized), 
                               ANIM_LERP(from->size.h, to->size.h, distance_normalized));
        property_animation->impl.accessors.setter.grect(property_animation->subject, new_rect);
    }
}

void property_animation_update_gpoint(PropertyAnimation * property_animation, const uint32_t distance_normalized)
{
    if (property_animation->impl.accessors.getter.gpoint != NULL && property_animation->impl.accessors.setter.gpoint != NULL)
    {
        GPoint *from = (GPoint *) property_animation->values.from;
        GPoint *to = (GPoint *) property_animation->values.to;
        
        GPoint new_origin = GPoint(ANIM_LERP(from->x, from->x, distance_normalized), 
                                   ANIM_LERP(from->y, to->y, distance_normalized));
        property_animation->impl.accessors.setter.gpoint(property_animation->subject, new_origin);
    }
}

void property_animation_update_int16(PropertyAnimation * property_animation, const uint32_t distance_normalized)
{
    if (property_animation->impl.accessors.getter.int16 != NULL && property_animation->impl.accessors.setter.int16 != NULL)
    {
        int16_t *from = (int16_t *) property_animation->values.from;
        int16_t *to = (int16_t *) property_animation->values.to;
        
        property_animation->impl.accessors.setter.int16(property_animation->subject, ANIM_LERP(*from, *to, distance_normalized));
    }
}

void property_animation_update_uint32(PropertyAnimation * property_animation, const uint32_t distance_normalized)
{
    if (property_animation->impl.accessors.getter.uint32 != NULL && property_animation->impl.accessors.setter.uint32 != NULL)
    {
        uint32_t *from = (uint32_t *) property_animation->values.from;
        uint32_t *to = (uint32_t *) property_animation->values.to;
        
        property_animation->impl.accessors.setter.uint32(property_animation->subject, ANIM_LERP(*from, *to, distance_normalized));
    }
}

void property_animation_update_gcolor8(PropertyAnimation * property_animation, const uint32_t distance_normalized)
{
    if (property_animation->impl.accessors.getter.gcolor != NULL && property_animation->impl.accessors.setter.gcolor != NULL)
    {
        GColor8 *from = (GColor8 *) property_animation->values.from;
        GColor8 *to = (GColor8 *) property_animation->values.to;
        
        GColor8 new_gcolor = *from;
        new_gcolor.r = ANIM_LERP(from->r, to->r, distance_normalized);
        new_gcolor.g = ANIM_LERP(from->g, to->g, distance_normalized);
        new_gcolor.b = ANIM_LERP(from->b, to->b, distance_normalized);
        new_gcolor.a = ANIM_LERP(from->a, to->a, distance_normalized);
        
        property_animation->impl.accessors.setter.gcolor(property_animation->subject, new_gcolor);
    }
}

static void property_animation_teardown(Animation * animation)
{
    property_animation_destroy((PropertyAnimation*) animation);
}

PropertyAnimation * property_animation_create_layer_frame(struct Layer * layer, GRect * from_frame, GRect * to_frame)
{
    const PropertyAnimationImplementation implementation = {
        .base = {
            .update = (AnimationUpdateImplementation) property_animation_update_grect,
            .teardown = (AnimationTeardownImplementation) property_animation_teardown,
        },
        .accessors = {
            .setter = { .grect = (GRectSetter) layer_set_frame },
            .getter = { .grect = (GRectGetter) layer_get_frame },
        },
    };
    
    PropertyAnimation *property_anim = property_animation_create(&implementation, layer, from_frame, to_frame);
    return property_anim;
}

PropertyAnimation * property_animation_create_bounds_origin(struct Layer * layer, GPoint * from, GPoint * to)
{
    const PropertyAnimationImplementation implementation = {
        .base = {
            .update = (AnimationUpdateImplementation) property_animation_update_gpoint,
            .teardown = (AnimationTeardownImplementation) property_animation_teardown,
        },
        .accessors = {
            .setter = { .gpoint = (GPointSetter) layer_set_bounds_origin },
            .getter = { .gpoint = (GPointGetter) layer_get_bounds_origin },
        },
    };
    
    PropertyAnimation *property_anim = property_animation_create(&implementation, layer, from, to);
    
    return property_anim;
}

PropertyAnimation * property_animation_create(const PropertyAnimationImplementation * implementation, void * subject, void * from_value, void * to_value)
{
    SYS_LOG("property_animation", APP_LOG_LEVEL_INFO, "property_animation_create");
    
    PropertyAnimation *property_animation = app_calloc(1, sizeof(PropertyAnimation));
    animation_ctor(&property_animation->animation);
    
    property_animation->animation.impl = implementation->base;
    property_animation->impl = *implementation;
    property_animation->subject = subject;
    
    // Set the values
    property_animation->values.from = from_value;
    property_animation->values.to = to_value;
    
    return property_animation;
}

Animation * property_animation_get_animation(PropertyAnimation * property_animation)
{
    return &property_animation->animation;
}

void property_animation_destroy(PropertyAnimation *property_animation)
{
    animation_dtor(&property_animation->animation);
    app_free(property_animation);
}

/*
 bool property_animation_subject(PropertyAnimation * property_animation, void * subject, bool set);
 bool property_animation_from(PropertyAnimation * property_animation, void * from, size_t size, bool set);
 bool property_animation_to(PropertyAnimation * property_animation, void * to, size_t size, bool set);
 */
