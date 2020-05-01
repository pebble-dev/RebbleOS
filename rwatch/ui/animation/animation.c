/* tick_timer_service.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 *         Barry Carter <barry.carter@gmail.com>
 */

/*
 * Notes
 * animation supports sequences, spawn and play counts.
 * Sequences can be made of sequences, so nodes are connected with a
 * linked list.
 * A sequence is denoted by the anim having a sequence_head != NULL
 * Each head should point to the first node.
 * Each node is connected to the next sequence_node
 * Node <-> ... next anim
 * Head  <----> Node <-> Node <-> NULL 
 * 
 * When we hit a sequence, the child anims are scheduled from sequence_head 
 * until exhaused.
 * The control is then returned to the head anim, which if it is a sequence
 * will move on to the next. and so on.
 * 
 * NOTE
 * although we use list_node struct, it is not being used as intended,
 * aS only the primitive is used. DO NOT use any node_list.h functions
 * It will likely break.
 * we do this becuase node_list requires a head element at all times.
 * We save the few bytes of memory by not requiring this, and instead 
 * traversing the linked list to the head/tail.
 * 
 * TODO
 * Infinite counts dont really work. No time spent on infinite sequences
 * Set elapsed on complex sequence spawns is not implemented
 */

#include "librebble.h"
#include "appmanager.h"
#include "FreeRTOS.h"
#include "animation.h"

/* Configure Logging */
#define MODULE_NAME "anim"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR


#define ANIMATION_FPS 60
#define ANIMATION_TICKS (pdMS_TO_TICKS(1000) / ANIMATION_FPS)

static Animation *_animation_play_next(Animation *anim);
static void _animation_update(Animation *anim);

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
    Animation *anim = app_calloc(1, sizeof(Animation));
    if (!anim) {
        LOG_ERROR("No Memory");
        return NULL;
    }
    animation_ctor(anim);

    LOG_DEBUG("[%x] animation_create", anim);
    return anim;
}

void animation_ctor(Animation* animation)
{
    list_init_node(&animation->sequence_node);
    list_init_node(&animation->sequence_head);
    animation->playcount = 1;
    animation->playcount_count = 0;
    animation->curve = AnimationCurveEaseInOut;

}

bool animation_destroy(Animation *anim)
{
    if (!anim)
        return false;

    LOG_DEBUG("[%x] animation_destroy", anim);

    animation_dtor(anim);
    app_free(anim);

    return true;
}

void animation_dtor(Animation* animation)
{
}

/* We use a double linked list from nose_list.h, but only the structure.
 * Node_list has the tail pointing back to the head, and vice versa. It has a 
 * requirement that the head node must be known at all times. Additionally,
 * node_list foreach macros use container_of logic on each iteration, when in 
 * fact we don't need the intermediate nodes, just head and tail.
 * We could store the head along with the node and point it to the real head
 * but that is really not ideal.
 * In this implementation we use the doubly linked list struct, and 
 * each end of the list is then NULL.
 * This is sub-optimal from a re-implementation and re-use point of view,
 * but sometimes...optimisation and marginal speed increase is worth it.
 * Terrible justification over.
 */
/* given a node, go and get the head element */
static list_node *_headless_list_get_root_head(list_node *node)
{
    if (!node || !node->prev)
        return NULL;

    while (node->prev) {
        node = node->prev;
    }

    return node;
}

/* given a node, go and get the tail element */
static list_node *_headless_list_get_root_tail(list_node *node)
{
    if (!node || !node->next)
        return NULL;

    while (node->next) {
        node = node->next;
    }
    assert(node->prev);
    return node;
}

/* callback invokations */

static void _animation_started(Animation *anim)
{
    if (!anim)
        return;

    anim->scheduled = 1;

    if (anim->impl.setup)
        anim->impl.setup(anim);

    if (anim->anim_handlers.started)
        anim->anim_handlers.started(anim, anim->context);
}

static bool _animation_stopped(Animation *anim)
{
    LOG_DEBUG("[%x] Animation stopped", anim);

    anim->playcount_count++;

    /* Ok, we're done. */
    if (anim->impl.update)
        anim->impl.update(anim, ANIMATION_NORMALIZED_MAX);

    if (_animation_play_next(anim))
        return false;

    anim->scheduled = 0;

    if (anim->anim_handlers.stopped)
        anim->anim_handlers.stopped(anim, true, anim->context);

    /* After teardown, the anim might not even exist anymore.  So don't
     * ever use it again afterwards!  */
    if (anim->impl.teardown)
        anim->impl.teardown(anim);

    return true;
}

static inline bool _is_play_complete(Animation *anim)
{
    return anim->playcount != ANIMATION_DURATION_INFINITE &&
        anim->playcount_count >= anim->playcount;
}

/* deal with the play count. If we are not finished, we reschedule */
static Animation *_animation_play_next(Animation *anim)
{
    if (!_is_play_complete(anim)) {
        anim->startticks = xTaskGetTickCount();
        anim->onqueue = 0;
        anim->scheduled = 0;

        animation_schedule(anim);
        LOG_INFO("[%x] Playing %d/%d", anim, anim->playcount_count + 1, anim->playcount);
        return anim;
    }
    return NULL;
}

/* When we are spawn, we cehck all timers are done before we move on */
static uint8_t _all_timers_complete(Animation *anim)
{
    anim = list_elem(anim->sequence_head.next, Animation, sequence_node);
    TickType_t now = xTaskGetTickCount();

    while (anim) {
        LOG_DEBUG("[%x] Checking complete w%d n%d %d %d %d %d", anim, anim->timer.when, now, anim->timer.when > now, anim->scheduled, anim->onqueue, anim->playcount_count < anim->playcount);
        if (anim->timer.when > now || anim->scheduled || 
            anim->onqueue || anim->playcount_count < anim->playcount)
            return 0;

        anim = list_elem(anim->sequence_node.next, Animation, sequence_node);
    }

    LOG_INFO("Done: Spawn timers report complete");

    /* we made it to the end. all timers report finished */
    return 1;
}

/* Called after each animation has timed out.
 * Deals with calling impl callbacks as well as plucking the
 * next timer to be executed in the sequence */
static void _animation_complete(Animation *anim)
{
    Animation *next = NULL;
    Animation *headanim = list_elem(_headless_list_get_root_head(&anim->sequence_node), Animation, sequence_head);

    if (!_animation_stopped(anim)) {
        return;
    }

    LOG_INFO("[%x] Complete", anim);

    /* If NOT part of a sequence, just cleanup and go */
    if (!headanim) {
//         anim->playcount_count = 0;
        return;
    }

    LOG_DEBUG("[%x] Processing Sequence", anim);

    /* We are a spawned sequence. Wait for all spawns to complete */
    if (headanim->spawn) {
        if (_all_timers_complete(headanim)) {
            LOG_DEBUG("[%x] Spawn Seq Complete", headanim);
            next->scheduled = 0;
            /* post the head back to the update so its callbacks are executed */
            _animation_update(headanim);
            return;
        }
        LOG_DEBUG("[%x] Waiting for all spawns...", anim);
        return;
    }

    /* if the previous node is a head node, and we are reversed... 
     * we are done */
    if (headanim->reverse) {
        if (anim->sequence_node.prev == &headanim->sequence_head)
            next = NULL;
        else
            next = list_elem(anim->sequence_node.prev, Animation, sequence_node);
    }
    else
        next = list_elem(anim->sequence_node.next, Animation, sequence_node);

    if (!next) {
        assert(headanim->sequence_head.next);
        LOG_INFO("[%x] Sequence Complete", headanim);
        _animation_update(headanim);
        return;
    }

    LOG_INFO("[%x] Scheduling Child anim: %x", anim, next);

    next->scheduled = 0;
    animation_schedule(next);
}

/* Update logic. This deals with a timer that has expired, or needs to be executed */
static void _animation_update(Animation *anim)
{
    app_running_thread *thread = appmanager_get_current_thread();

    if (anim->onqueue)
        appmanager_timer_remove(&thread->timer_head, &anim->timer);

    anim->onqueue = 0;
    TickType_t now = xTaskGetTickCount();
    TickType_t progress = now - anim->startticks;

    if (anim->duration == ANIMATION_DURATION_INFINITE) 
    {
        if (anim->sequence_head.next) {
            /* If we an infinite head element, just scedule us again */
            animation_schedule(anim);
            return;
        }
        anim->timer.when = now + ANIMATION_TICKS;
        
        appmanager_timer_add(&thread->timer_head, &anim->timer);
        return;
    }

    if (progress > anim->duration) {
        /* We are done */
        LOG_DEBUG("[%x] Animation Progress Done", anim);

        _animation_complete(anim);
        return;
    }
    anim->timer.when = now + ANIMATION_TICKS;
    progress *= ANIMATION_NORMALIZED_MAX;
    progress /= anim->duration;

    /* deal with the animation being reversed */
    /* If the sequence is reversed, then the child elements are reversed
     * unless they themselves are reversed, in which case its forwards. */
    bool reverse = false;
    if (anim->sequence_node.next || anim->sequence_node.prev) {
        Animation *headanim = list_elem(_headless_list_get_root_head(&anim->sequence_node), Animation, sequence_head);
        reverse = headanim->reverse;
        if (reverse)
            reverse = !anim->reverse;
    }
    else {
        reverse = anim->reverse;
    }
    progress = reverse ? ANIMATION_NORMALIZED_MAX - progress  : progress;

    switch(anim->curve) {
        case AnimationCurveDefault:
        case AnimationCurveLinear:
            break;
        case AnimationCurveEaseIn:
            progress = ((progress * progress) / ANIMATION_NORMALIZED_MAX);
            break;
        case AnimationCurveEaseOut:
            progress -= ANIMATION_NORMALIZED_MAX;
            progress = -(progress * progress / ANIMATION_NORMALIZED_MAX) + ANIMATION_NORMALIZED_MAX;
            break;
        case AnimationCurveEaseInOut:
            if (progress < ANIMATION_NORMALIZED_MAX / 2){
                progress = ((progress * progress) / (ANIMATION_NORMALIZED_MAX / 2));
            }
            else {
                progress -= ANIMATION_NORMALIZED_MAX;
                progress = -((progress * progress) / (ANIMATION_NORMALIZED_MAX / 2)) + ANIMATION_NORMALIZED_MAX;
            }
            break;
        case AnimationCurveCustomFunction:
            if (anim->curve_function)
                progress = anim->curve_function(progress);
            break;
        case AnimationCurveCustomInterpolationFunction:
        case AnimationCurve_Reserved1:
        case AnimationCurve_Reserved2:
            break;
    }

    if (anim->impl.update)
        anim->impl.update(anim, (uint32_t) progress);

    anim->onqueue = 1;
    appmanager_timer_add(&thread->timer_head, &anim->timer);
}

static void _anim_callback(CoreTimer *timer)
{
    Animation *anim = (Animation *) timer;

    anim->onqueue = 0;
    _animation_update(anim);
}

/* Anims added to a sequence are not allowed to be changed.
 * Same with scheduled anims */
static inline bool _is_immutable(Animation *anim)
{
    if (!anim || anim->sequence_node.next || 
        anim->sequence_node.prev || 
        anim->scheduled || anim->onqueue)
        return true;

    return false;
}

static bool _animation_schedule_one(Animation *anim)
{
    app_running_thread *thread = appmanager_get_current_thread();
    if (anim->scheduled)
        assert(0);

    LOG_INFO("[%x] Animation scheduled", anim);

    _animation_started(anim);

    if (anim->onqueue)
        appmanager_timer_remove(&thread->timer_head, &anim->timer);

    anim->startticks = xTaskGetTickCount();
    anim->scheduled = 1;
    anim->onqueue = 0;
    anim->timer.when = 0;
    anim->timer.callback = _anim_callback;

    /* If we are delaying, add the timer to the queue.
       when it times out it will call update*/
    if (anim->delay > 0) {
        LOG_INFO("[%x] Delay %d", anim, anim->delay);
        anim->timer.when = anim->startticks + anim->delay;
        anim->startticks = anim->timer.when;
        anim->onqueue = 1;
        appmanager_timer_add(&thread->timer_head, &anim->timer);
        return true;
    }

    /* We are a head element of a sequence 
     * get the next child, notify start, and schedule */
    if (anim->sequence_head.next) {
        if (anim->reverse)
            anim = list_elem(_headless_list_get_root_tail(&anim->sequence_head), Animation, sequence_node);
        else
            anim = list_elem(anim->sequence_head.next, Animation, sequence_node);

        _animation_started(anim);
    }

    _animation_update(anim);

    return true;
}

bool animation_schedule(Animation *anim)
{
    if (!anim || anim->scheduled)
        return false;

    LOG_DEBUG("[%x] Animation schedule: when %d", anim, anim->timer.when);

    /* Spawned anims all run at once, so schedule them all */
    if (anim->spawn) {
        list_node *sequence_members = anim->sequence_head.next;

        while (anim) {
            anim = list_elem(sequence_members, Animation, sequence_node);
            if (!anim)
                break;

            animation_schedule(anim);
            sequence_members = anim->sequence_node.next;
        }
        return true;
    }
    _animation_schedule_one(anim);

    return true;
}

bool animation_unschedule(Animation *anim)
{
    LOG_INFO("[%x] Animation unscheduled", anim);

    if (!anim || !anim->scheduled)
        return true;

    anim->scheduled = 0;
    anim->onqueue = 0;

    if (anim->anim_handlers.stopped)
        anim->anim_handlers.stopped(anim, false, anim->context);

    anim->playcount_count = 0;

    /* After teardown, the anim might not even exist anymore.  So don't
     * ever use it again afterwards!  */
    if (anim->impl.teardown)
        anim->impl.teardown(anim);

    LOG_INFO("[%x] Animation unscheduled END", anim);
    return true;
}

void animation_unschedule_all(void)
{
    LOG_ERROR("Can't unschedule!");
    /* actually we can't do this! yet... Handles! */
}

bool animation_is_scheduled(Animation *anim)
{
    if (!anim)
        return false;

    return anim->scheduled == 1;
}

void animation_set_delay(Animation *anim, uint32_t delay)
{
    if (_is_immutable(anim))
        return;

    anim->delay = pdMS_TO_TICKS(delay);
}

uint32_t animation_get_delay(Animation *anim)
{
    if (!anim)
        return 0;

    return anim->delay / configTICK_RATE_HZ;
}

uint32_t animation_get_duration(Animation *anim, bool include_delay, bool include_play_count)
{
    /* TODO does not work on sequences */
    if (!anim)
        return 0;

    uint32_t duration = anim->duration / configTICK_RATE_HZ;
    uint32_t delay = anim->delay / configTICK_RATE_HZ;

    if (include_play_count)
        duration = (anim->playcount * duration);

    if (include_delay)
        duration += delay;

    return duration;
}

bool animation_set_duration(Animation *anim, uint32_t ms)
{
    /* TODO does not work on sequences */
    if (_is_immutable(anim))
        return false;

    if (ms == ANIMATION_DURATION_INFINITE)
        anim->duration = ANIMATION_DURATION_INFINITE;
    else
        anim->duration = pdMS_TO_TICKS(ms);
    return true;
}

bool animation_set_elapsed(Animation *anim, uint32_t elapsed_ms)
{
    /* TODO does not work on sequences */
    if (!anim || !elapsed_ms)
        return false;
    if (_is_immutable(anim))
        return false;

    TickType_t now = xTaskGetTickCount();
    TickType_t progress = (now - anim->startticks) / configTICK_RATE_HZ;

    /* TODO if it is a sequence we have to find the right sequence inside it */
    /* a spawn: get the longest and check validity, then fast forward all */

    if (elapsed_ms < progress)
        return false;

    uint32_t diff = pdMS_TO_TICKS(elapsed_ms - progress);
    anim->startticks -= diff;
    anim->timer.when = diff;

    return true;
}

bool animation_get_elapsed(Animation *anim, int32_t *elapsed_ms)
{
    if (!anim || !elapsed_ms)
        return false;

    TickType_t now = xTaskGetTickCount();
    TickType_t progress = (now - anim->startticks) / configTICK_RATE_HZ;

    *elapsed_ms = progress;

    return true;
}

bool animation_set_reverse(Animation *anim, bool reverse)
{
    if (_is_immutable(anim))
        return false;

    anim->reverse = (uint8_t)reverse;
    return true;
}

bool animation_get_reverse(Animation *anim)
{
    return anim->reverse == 1;
}

bool animation_set_play_count(Animation *anim, uint32_t play_count)
{
    if (!anim || !play_count)
        return false;

    if (_is_immutable(anim))
        return false;

    anim->playcount = play_count;

    return true;
}

uint32_t animation_get_play_count(Animation *anim)
{
    if (!anim)
        return 0;

    return anim->playcount;
}

void animation_set_curve(Animation *anim, uint8_t animation_curve)
{
    if (_is_immutable(anim))
        return;

    anim->curve = animation_curve;
}

AnimationCurve animation_get_curve(Animation *anim)
{
    if (!anim)
        return 0;

    return anim->curve;
}

bool animation_set_custom_curve(Animation * anim, AnimationCurveFunction curve_function)
{
    if (!anim || !curve_function)
        return false;

    if (_is_immutable(anim))
        return false;

    MK_THUMB_CB(anim->curve_function);

    return true;
}

AnimationCurveFunction animation_get_custom_curve(Animation *anim)
{
    if (!anim)
        return NULL;

    return anim->curve_function;
}


bool animation_set_implementation(Animation *anim, const AnimationImplementation *impl)
{
    if (!anim || !impl)
        return false;

    if (_is_immutable(anim))
        return false;

    anim->impl = *impl;
    if (anim->impl.setup)
        MK_THUMB_CB(anim->impl.setup);
    if (anim->impl.update)
        MK_THUMB_CB(anim->impl.update);
    if (anim->impl.teardown)
        MK_THUMB_CB(anim->impl.teardown);

    return true;
}

const AnimationImplementation *animation_get_implementation(Animation *anim)
{
    if (!anim)
        return NULL;

    return &anim->impl;
}

Animation *animation_clone(Animation *from)
{
    /* TODO sequences */
    Animation *newanim = app_calloc(1, sizeof(Animation));
    memcpy(newanim, from, sizeof(Animation));
    newanim->scheduled = 0;
    newanim->onqueue = 0;
    newanim->playcount_count = 0;
    return newanim;
}

void animation_set_handlers(Animation *anim, AnimationHandlers anim_handlers, void *context)
{
    if (!anim)
        return;

    if (_is_immutable(anim))
        return;

    anim->context = context;
    if (anim_handlers.started)
        anim->anim_handlers.started = (void *)(((uint32_t)anim_handlers.started) | 1);
    if (anim_handlers.stopped)
        anim->anim_handlers.stopped = (void *)(((uint32_t)anim_handlers.stopped) | 1);
}

void *animation_get_context(Animation *anim)
{
    if (!anim)
        return NULL;

    return anim->context;
}

void animation_set_context(Animation *anim, void *context)
{
    if (!anim)
        return;

    anim->context = context;
}

static void _append_link_child(Animation *appendto, Animation *anim)
{
    if (appendto->sequence_node.next != NULL)
        return;

    anim->sequence_node.prev = &appendto->sequence_node;
    appendto->sequence_node.next = &anim->sequence_node;
    anim->sequence_node.next = NULL;
}

Animation *_animation_sequence_create_v(Animation *animation_a, Animation *animation_b, 
                                        Animation *animation_c, va_list ar, uint8_t is_spawn)
{
    if (!animation_a && !animation_b) {
        LOG_ERROR("Anim A or B is invalid");
        return NULL;
    }

    /* Create the new animation that is the head element */
    Animation *head_anim = animation_create();

    if (!head_anim)
        return NULL;

    /* init the head */
    head_anim->sequence_head.prev = NULL;
    head_anim->sequence_head.next = &animation_a->sequence_node;
    animation_a->sequence_node.prev = &head_anim->sequence_head;

    _append_link_child(animation_a, animation_b);

    head_anim->spawn = is_spawn;

    if (!animation_c)
        return head_anim;

    _append_link_child(animation_b, animation_c);

    /* Loop through each animation and link it to the last */
    Animation *anim;
    list_node *tail = &animation_c->sequence_node;
;
    for (uint8_t i = 0; i < 20; i++) {
        anim = va_arg(ar, Animation *);
        if (!anim)
            break;

        _append_link_child(list_elem(tail, Animation, sequence_node), anim);
        tail = tail->next;
        tail->next = NULL;
    }

    return head_anim;
}

Animation *animation_sequence_create(Animation *animation_a, Animation *animation_b, Animation *animation_c, ...)
{
    va_list ar;
    va_start(ar, animation_c);
    Animation *anim = _animation_sequence_create_v(animation_a, animation_b, animation_c, ar, 0);
    va_end(ar);

    return anim;
}

Animation *animation_sequence_create_from_array(Animation ** animation_array, uint32_t array_len)
{
    Animation *anim;
    Animation *lastanim = NULL;

    if (array_len < 3 || array_len > 255)
        return NULL;

    /* Create the new animation that is the head element */
    Animation *head_anim = animation_create();

    if (!head_anim)
        return NULL;

    /* init the head */
    head_anim->sequence_head.prev = NULL;
    head_anim->sequence_head.next = &animation_array[0]->sequence_node;
    animation_array[0]->sequence_node.prev = &head_anim->sequence_head;
    uint8_t i = 0;
    for (i = 1; i < array_len; i++) {
        anim = animation_array[i];
        if (!anim)
            break;
        _append_link_child(animation_array[i-1], anim);
    }
    animation_array[i]->sequence_node.next = NULL;

    return head_anim;
}

Animation *animation_spawn_create(Animation *animation_a, Animation *animation_b, Animation *animation_c, ...)
{
    va_list ar;
    va_start(ar, animation_c);

    Animation *a = _animation_sequence_create_v(animation_a, animation_b, animation_c, ar, 1);
    va_end(ar);

    return a;
}

Animation *animation_spawn_create_from_array(Animation ** animation_array, uint32_t array_len)
{
    Animation *anim = animation_sequence_create_from_array(animation_array, array_len);
    anim->spawn = true;
    return anim;
}

