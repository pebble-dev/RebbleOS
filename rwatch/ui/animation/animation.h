#pragma once
/* animation.h
 * declarations for animations
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"

#include "librebble.h"
#include "point.h"
#include "rect.h"
#include "size.h"

#include "appmanager.h"

#define ANIMATION_DURATION_INFINITE UINT32_MAX
#define ANIMATION_NORMALIZED_MIN 0 
#define ANIMATION_NORMALIZED_MAX 65535

struct Animation;

typedef enum {
    AnimationCurveLinear,
    AnimationCurveEaseIn,
    AnimationCurveEaseOut,
    AnimationCurveEaseInOut,
    AnimationCurveDefault,
    AnimationCurveCustomFunction,
    AnimationCurveCustomInterpolationFunction,
    AnimationCurve_Reserved1,
    AnimationCurve_Reserved2
} AnimationCurve;

typedef void (*AnimationSetupImplementation)(struct Animation *animation);
typedef void (*AnimationUpdateImplementation)(struct Animation *animation, const uint32_t time_normalized);
typedef void (*AnimationTeardownImplementation)(struct Animation *animation);

typedef struct AnimationImplementation
{
    AnimationSetupImplementation setup;
    AnimationUpdateImplementation update;
    AnimationTeardownImplementation teardown;
} AnimationImplementation;

typedef void (*AnimationStartedHandler)(struct Animation *animation, void *context);
typedef void (*AnimationStoppedHandler)(struct Animation *animation, bool finished, void *context);

// animation
typedef struct Animation
{
    AppTimer timer;
    
    int scheduled;
    int onqueue;
    TickType_t duration;
    TickType_t startticks;
    AnimationImplementation impl;
    struct AnimationHandler *anim_handlers;
} Animation;

typedef struct AnimationHandler
{
    void *started;
    void *stopped;
} AnimationHandler;

typedef int32_t AnimationProgress;
typedef AnimationProgress(* AnimationCurveFunction)(AnimationProgress linear_distance);

// animation
Animation *animation_create();
bool animation_destroy(Animation *animation);
Animation *animation_clone(Animation *from);
Animation *animation_sequence_create(Animation *animation_a, Animation *animation_b, Animation *animation_c, ...);
Animation *animation_sequence_create_from_array(Animation ** animation_array, uint32_t array_len);
Animation *animation_spawn_create(Animation *animation_a, Animation *animation_b, Animation *animation_c, ...);
Animation *animation_spawn_create_from_array(Animation ** animation_array, uint32_t array_len);
bool animation_set_elapsed(Animation *animation, uint32_t elapsed_ms);
bool animation_get_elapsed(Animation *animation, int32_t *elapsed_ms);
bool animation_set_reverse(Animation *animation, bool reverse);
bool animation_get_reverse(Animation *animation);
bool animation_set_play_count(Animation *animation, uint32_t play_count);
uint32_t animation_get_play_count(Animation *animation);
bool animation_set_duration(Animation *animation, uint32_t duration_ms);
uint32_t animation_get_duration(Animation *animation, bool include_delay, bool include_play_count);
void animation_set_delay(Animation *anim, uint32_t delay);
uint32_t animation_get_delay(Animation *animation);
void animation_set_curve(Animation *anim, uint8_t animation_curve);
AnimationCurve animation_get_curve(Animation *animation);
AnimationCurveFunction animation_get_custom_curve(Animation *animation);
bool animation_set_implementation(Animation *animation, const AnimationImplementation *implementation);
const AnimationImplementation *animation_get_implementation(Animation *animation);
void animation_set_handlers(Animation *anim, AnimationHandler anim_handler);
void *animation_get_context(Animation *animation);
bool animation_schedule(Animation *anim);
bool animation_unschedule(Animation *animation);
void animation_unschedule_all(void);
bool animation_is_scheduled(Animation *animation);

typedef GPoint GPointReturn;
typedef GRect GRectReturn;
typedef void (*Int16Setter)(void *subject, int16_t int16);
typedef void (*GPointSetter)(void *subject, GPoint gpoint);
typedef void (*GRectSetter)(void *subject, GRect grect);
typedef int16_t (*Int16Getter)(void *subject);
typedef GPointReturn (*GPointGetter)(void *subject);
typedef GRectReturn (*GRectGetter)(void *subject);

typedef struct PropertyAnimationAccessors
{
    union 
    {
        Int16Setter int16;
        GPointSetter gpoint;
        GRectSetter grect;
    } setter;
    union 
    {
        Int16Getter int16;
        GPointGetter gpoint;
        GRectGetter grect;
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
        union 
        {
            GRect grect;
            GPoint gpoint;
            int16_t int16;
        } to;
        union 
        {
            GRect grect;
            GPoint gpoint;
            int16_t int16;
        } from;
    } values;
    void *subject;
} PropertyAnimation;
