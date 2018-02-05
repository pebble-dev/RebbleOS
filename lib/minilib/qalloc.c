/* qalloc.c
 * Fixed-heap allocator
 *
 * Author: Elizabeth Fong-Jones <elly@leptoquark.net>
 * Public domain; optionally see LICENSE
 */

#include <minilib.h>
#include <qalloc.h>

#define FFREE	0x00000001
#define MINBSZ	4

#define SZ(ptr)		*((unsigned*)ptr)
#define BSZ(p)		(SZ(p) & ~3)
#define NX(ptr)		(void*)((char*)ptr + BSZ(ptr))
#define ALIGN(s)	((s + 3) & ~3)
#define RNDSZ(s)	(ALIGN(s) + 4)
#define UPTR(p)		(void*)((char*)p + sizeof(unsigned))
#define BPTR(p)		(void*)((char*)p - sizeof(unsigned))

#define ISFREE(p)	(SZ(p) & FFREE)
#define FREE(p)		SZ(p) |= FFREE
#define ALLOC(p)	SZ(p) &= ~FFREE;

static void qjoin(qarena_t *arena);

qarena_t *qinit(void *start, unsigned size) {
	qarena_t *arena = start;
	arena->size = size;
	start = arena+1; // start = &arena[1], so arena[0] is left alone.
	SZ(start) = size - sizeof(*arena);
	FREE(start);
	return arena;
}

void *qalloc(qarena_t *arena, unsigned size) {
	void *p = arena+1;
	void *end = (char *)arena + arena->size;
	void *n = NULL;

	size = RNDSZ(size);
	
	while (p && p < end) {
		if (!ISFREE(p) || BSZ(p) < size) {
			p = NX(p);
			continue;
		}

		if (BSZ(p) > size) {
			n = (char*)p + size;
			SZ(n) = SZ(p) - size;
			SZ(p) = size;
		}

		ALLOC(p);
		return UPTR(p);
	}
	return NULL;
}

void qfree(qarena_t *arena, void *ptr) {
	FREE(BPTR(ptr));
	qjoin(arena);
}
static void qjoin(qarena_t *arena) {
	void *p = arena+1;
	void *end = (char *)arena + arena->size;
	while (p && p < end) {
		if (NX(p) < end && ISFREE(p) && ISFREE(NX(p))) {
			SZ(p) += BSZ(NX(p));
		} else {
			p = NX(p);
		}
	}
}
