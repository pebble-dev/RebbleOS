#pragma once
/* rtoswrap.h
 * Wrappers for static allocation of threads and queues.
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "task.h" /* xTaskCreate */
#include "queue.h" /* xQueueCreate */

#define THREAD_DEFINE(name, stack_size, priority, entry) \
    static TaskHandle_t _##name##_task; \
    static StaticTask_t _##name##_task_buf; \
    static StackType_t  _##name##_task_stack[stack_size]; \
    static const TaskFunction_t _##name##_task_entry = &(entry); \
    static const int _##name##_task_prio = priority

#define THREAD_HANDLE(name) _##name##_task

#define THREAD_CREATE(name) _##name##_task = xTaskCreateStatic(_##name##_task_entry, #name, sizeof(_##name##_task_stack) / sizeof(StackType_t), NULL, _##name##_task_prio, _##name##_task_stack, &_##name##_task_buf)

#define QUEUE_DEFINE(name, type, depth) \
    static xQueueHandle _##name##_queue; \
    static StaticQueue_t _##name##_queue_buf; \
    static type _##name##_queue_contents[depth]

#define QUEUE_HANDLE(name) _##name##_queue

#define QUEUE_CREATE(name) _##name##_queue = xQueueCreateStatic(sizeof(_##name##_queue_contents) / sizeof(_##name##_queue_contents[0]), sizeof(_##name##_queue_contents[0]), (uint8_t *) _##name##_queue_contents, &_##name##_queue_buf)
