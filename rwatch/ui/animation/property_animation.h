#pragma once
/* property_animation.h
 * declarations for property animations
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "FreeRTOS.h"

#include "librebble.h"
#include "point.h"
#include "rect.h"
#include "size.h"
#include "animation.h"

#include "appmanager.h"

typedef GPoint GPointReturn;
typedef GRect GRectReturn;
typedef void (*Int16Setter)(void *subject, int16_t int16);
typedef void (*UInt32Setter)(void *subject, uint32_t int32);
typedef void (*GPointSetter)(void *subject, GPoint gpoint);
typedef void (*GRectSetter)(void *subject, GRect grect);
typedef void (*GColor8Setter)(void *subject, GColor8 gcolor);
typedef int16_t (*Int16Getter)(void *subject);
typedef uint32_t (*UInt32Getter)(void *subject);
typedef GPointReturn (*GPointGetter)(void *subject);
typedef GRectReturn (*GRectGetter)(void *subject);
typedef GColor8 (*GColor8Getter)(void *subject);

typedef struct PropertyAnimationAccessors
{
    union 
    {
        Int16Setter int16;
        UInt32Setter uint32;
        GPointSetter gpoint;
        GRectSetter grect;
        GColor8Setter gcolor;
    } setter;
    union 
    {
        Int16Getter int16;
        UInt32Getter uint32;
        GPointGetter gpoint;
        GRectGetter grect;
        GColor8Getter gcolor;
    } getter;
} PropertyAnimationAccessors;

typedef struct PropertyAnimationImplementation
{
    AnimationImplementation base;
    PropertyAnimationAccessors accessors;
} PropertyAnimationImplementation;

typedef struct PropertyAnimation
{
    Animation animation;
    struct
    {
        void *from;
        void *to;
    } values;
    
    PropertyAnimationImplementation impl;
    void *subject;
} PropertyAnimation;

PropertyAnimation * property_animation_create_layer_frame(struct Layer * layer, GRect * from_frame, GRect * to_frame);
PropertyAnimation * property_animation_create_bounds_origin(struct Layer * layer, GPoint * from, GPoint * to);
PropertyAnimation * property_animation_create(const PropertyAnimationImplementation * implementation, void * subject, void * from_value, void * to_value);
void property_animation_destroy(PropertyAnimation *property_animation);
void property_animation_update_int16(PropertyAnimation * property_animation, const uint32_t distance_normalized);
void property_animation_update_uint32(PropertyAnimation * property_animation, const uint32_t distance_normalized);
void property_animation_update_gpoint(PropertyAnimation * property_animation, const uint32_t distance_normalized);
void property_animation_update_grect(PropertyAnimation * property_animation, const uint32_t distance_normalized);
void property_animation_update_gcolor8(PropertyAnimation * property_animation, const uint32_t distance_normalized);
Animation * property_animation_get_animation(PropertyAnimation * property_animation);
bool property_animation_subject(PropertyAnimation * property_animation, void * subject, bool set);
bool property_animation_from(PropertyAnimation * property_animation, void * from, size_t size, bool set);
bool property_animation_to(PropertyAnimation * property_animation, void * to, size_t size, bool set);
