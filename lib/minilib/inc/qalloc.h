/* qalloc.h
 * Fixed-heap allocator
 *
 * Author: Elizabeth Fong-Jones <elly@leptoquark.net>
 * Public domain; optionally see LICENSE
 */

#ifndef QALLOC_H
#define QALLOC_H

typedef struct _qarena_t {
	unsigned int size;
	/* ... */
} qarena_t;

extern qarena_t *qinit(void *start, unsigned size);
extern void *qalloc(qarena_t *arena, unsigned size);
extern void qfree(qarena_t *arena, void *ptr);

#endif /* !QALLOC_H */
