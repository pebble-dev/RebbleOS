/* tick_timer_service.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "librebble.h"
#include "appmanager.h"
#include "FreeRTOS.h"

#define ANIMATION_FPS 30
#define ANIMATION_TICKS (pdMS_TO_TICKS(1000) / ANIMATION_FPS)

/* XXX: The memory allocation story here is kind of a mess.  We do an
 * app_malloc on this, and store a bunch of state in the application's
 * memory -- you know, where the application could trample on it.  This is
 * widely considered to be bad.  I think the best way to deal with this is
 * to come up with a 'handle' object stored in the App structure, which is
 * allocated in system memory; then, to look up a handle and verify it, you
 * walk through the handles in the App structure to make sure that one is
 * permitted to be used.  If an app is killed, then you simply go walk
 * through all the handles and free them.
 *
 * But in the mean time, we're here.
 */

Animation *animation_create()
{
    SYS_LOG("animation", APP_LOG_LEVEL_INFO, "animation_create");
    
    Animation *anim = app_calloc(1, sizeof(Animation));
    
    return anim;
}

bool animation_destroy(Animation *anim)
{
    if (!anim)
        return false;
    
    return true;
}

void _animation_update(Animation *anim)
{
    if (anim->onqueue)
        appmanager_timer_remove(&anim->timer);
    
    TickType_t now = xTaskGetTickCount();
    
    anim->timer.when = now + ANIMATION_TICKS;
    if (anim->duration == ANIMATION_DURATION_INFINITE) {
        anim->onqueue = 1;
        appmanager_timer_add(&anim->timer);
        return;
    }
    
    TickType_t progress = now - anim->startticks;
    if (progress > anim->duration) {
        /* Ok, we're done. */
        if (anim->impl.update)
            anim->impl.update(anim, ANIMATION_NORMALIZED_MAX);
        if (anim->impl.teardown)
            anim->impl.teardown(anim);
        anim->scheduled = 0;
        return;
    }
    
    progress *= ANIMATION_NORMALIZED_MAX;
    progress /= anim->duration;
    
    if (anim->impl.update)
        anim->impl.update(anim, (uint32_t) progress);
    
    anim->onqueue = 1;
    appmanager_timer_add(&anim->timer);
}

void _anim_callback(CoreTimer *timer)
{
    Animation *anim = (Animation *) timer;
    
    anim->onqueue = 0;
    _animation_update(anim);
}

bool animation_schedule(Animation *anim)
{
    SYS_LOG("animation", APP_LOG_LEVEL_INFO, "animation scheduled");
    if (anim->impl.setup)
        anim->impl.setup(anim);
    
    if (anim->onqueue)
        appmanager_timer_remove(&anim->timer);
        
    anim->startticks = xTaskGetTickCount();
    anim->scheduled = 1;
    
    anim->timer.callback = _anim_callback;
    _animation_update(anim);
    
    return true;
}

bool animation_set_duration(Animation *anim, uint32_t ms)
{
    if (!anim)
        return false;
        
    if (ms == ANIMATION_DURATION_INFINITE)
        anim->duration = ANIMATION_DURATION_INFINITE;
    else
        anim->duration = pdMS_TO_TICKS(ms);
    return true;
}

bool animation_set_implementation(Animation *anim, const AnimationImplementation *impl)
{
    if (!anim)
        return false;
    
    anim->impl = *impl;
    if (anim->impl.setup)
        anim->impl.setup = (void *)(((uint32_t)anim->impl.setup) | 1);
    if (anim->impl.update)
        anim->impl.update = (void *)(((uint32_t)anim->impl.update) | 1);
    if (anim->impl.teardown)
        anim->impl.teardown = (void *)(((uint32_t)anim->impl.teardown) | 1);

    return true;
}

