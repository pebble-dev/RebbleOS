/* qalloc.c
 * Fixed-heap allocator
 *
 * Author: Elizabeth Fong-Jones <elly@leptoquark.net>
 * Public domain; optionally see LICENSE
 */

#include <minilib.h>
#include <qalloc.h>
#include <debug.h>

/* XXX: when we have debug/release builds, we should have this only in debug builds */
#define HEAP_INTEGRITY
//#define HEAP_PARANOID

#define MINBSZ	4

#define SZFLAG_SZ (~3)
#define SZFLAG_FFREE 1


typedef struct qblock {
#ifdef HEAP_INTEGRITY
	unsigned long cookie0;
#endif
	unsigned long szflag;
#ifdef HEAP_INTEGRITY
	unsigned long cookie1;
#endif
} qblock_t;

#define ALIGN(s)	((s + 3) & ~3)

#define BLK(blk)		((qblock_t *)(blk))
#define BLK_FROMPAYLOAD(p)	(void*)((char*)(p) - sizeof(qblock_t))
#define BLK_SZ(blk)	((blk)->szflag & SZFLAG_SZ)
#define BLK_NEXT(blk)	((qblock_t *)((char*)(blk) + BLK_SZ(blk)))
#define BLK_ISFREE(blk)	((blk)->szflag & SZFLAG_FFREE)
#define BLK_FREE(blk)	((blk)->szflag |= SZFLAG_FFREE)
#define BLK_ALLOC(blk)	((blk)->szflag &= ~SZFLAG_FFREE)
#define BLK_PAYLOAD(p)	(void*)((char*)(p) + sizeof(qblock_t))
#define BLK_COOKIE(arena, blk) ((unsigned)(arena) >> 4 ^ (unsigned)(blk))
#define BLK_ALSIZE(size) ALIGN(size) + sizeof(qblock_t);

static void _cookie_set(qarena_t *arena, qblock_t *blk) {
#ifdef HEAP_INTEGRITY
	blk->cookie0 = ~BLK_COOKIE(arena, blk);
	blk->cookie1 = BLK_COOKIE(arena, blk);
#endif
}
static void _cookie_unset(qarena_t *arena, qblock_t *blk) {
#ifdef HEAP_INTEGRITY
	blk->cookie0 = BLK_COOKIE(arena, blk);
	blk->cookie1 = ~BLK_COOKIE(arena, blk);
#endif
}

static void qjoin(qarena_t *arena);
static void qcheck(qarena_t *arena, qblock_t *blk);
static void _qsplit(qarena_t *arena, qblock_t *blk, unsigned size);

qarena_t *qinit(void *start, unsigned size) {
	qarena_t *arena = start;
	arena->size = size;
	
	qblock_t *blk = BLK(arena + 1); // start = &arena[1], so arena[0] is left alone.
	blk->szflag = size - sizeof(*arena);
		_cookie_unset(arena, blk);
#ifdef HEAP_PARANOID
	memset(BLK_PAYLOAD(blk), 0xAA, BLK_SZ(blk) - sizeof(qblock_t));
#endif
	BLK_FREE(blk);
	
	return arena;
}

void *qalloc(qarena_t *arena, unsigned size) {
	qblock_t *blk = BLK(arena+1);
	qblock_t *end = BLK((char *)arena + arena->size);
	void *n = NULL;
	
	if (size == 0)
		return NULL;

	size = BLK_ALSIZE(size);
	
	while (blk && blk < end) {
		qcheck(arena, blk);
		
		/* We need either exactly enough room for this block, or we
		 * need enough room for this and one more block to be
		 * constructed at the end.  */
		if (!BLK_ISFREE(blk) ||
			((BLK_SZ(blk) != size) &&
			 (BLK_SZ(blk) < size + sizeof(qblock_t)))) {
			blk = BLK_NEXT(blk);
			continue;
		}

		if (BLK_SZ(blk) > size) {
			_qsplit(arena, blk, size);
		}

		_cookie_set(arena, blk);
		BLK_ALLOC(blk);
		
		return BLK_PAYLOAD(blk);
	}
	return NULL;
}

/* init a new block, carving the remaining space into a new free */
static void _qsplit(qarena_t *arena, qblock_t *blk, unsigned size) {
	qblock_t *newblk = (qblock_t *)((char*)blk + size);
	newblk->szflag = BLK_SZ(blk) - size;
	blk->szflag = size;
	_cookie_unset(arena, newblk);
	BLK_FREE(newblk);
}

void *qrealloc(qarena_t *arena, void *ptr, unsigned size) {
	if (size == 0)
		return NULL;
	
	size = BLK_ALSIZE(size);
	
	if (!ptr)
		return qalloc(arena, size);
	
	qblock_t *blk = BLK_FROMPAYLOAD(ptr);
	
	/* is the new size smaller? */
	if (size < BLK_SZ(blk)) {
		_qsplit(arena, blk, size);
		_cookie_unset(arena, blk);
		qjoin(arena);
		return BLK_PAYLOAD(blk);
	}
	
	/* is there room after */
	qblock_t *nblk = BLK_NEXT(blk);
	uint32_t reqsize = size - BLK_SZ(blk);
	if (BLK_SZ(nblk) >= reqsize) {
		/* there is a free block after. Lets take what we need */
		_qsplit(arena, nblk, reqsize);
		blk->szflag = size;
		return BLK_PAYLOAD(blk);
	}

	/* There is no room after. Try malloc */
	void *newm = qalloc(arena, size);
	if (!newm)
		return NULL;
	
	memcpy(newm, BLK_PAYLOAD(blk), BLK_SZ(blk));
	qfree(arena, BLK_PAYLOAD(blk));
	qjoin(arena);
	
	return newm;
}

uint32_t qusedbytes(qarena_t *arena) {
	qblock_t *blk = BLK(arena+1);
	qblock_t *end = BLK((char *)arena + arena->size);
	uint32_t cnt = 0;
	
	while (blk && blk < end) {
		if (!BLK_ISFREE(blk)) {
			cnt += BLK_SZ(blk);
		}
		blk = BLK_NEXT(blk);
	}
	return cnt;
}

uint32_t qfreebytes(qarena_t *arena) {
	return arena->size - qusedbytes(arena);
}

void qfree(qarena_t *arena, void *ptr) {
	if (!ptr)
		return;
		
	qblock_t *blk = BLK_FROMPAYLOAD(ptr);

#ifdef HEAP_INTEGRITY
	qcheck(arena, blk);
	if (BLK_ISFREE(blk))
		panic("qfree: double free");	/* XXX: this "panic" needs to not panic if we are in an app */
#endif
#ifdef HEAP_PARANOID
	memset(BLK_PAYLOAD(blk), 0xAA, BLK_SZ(blk) - sizeof(qblock_t));
#endif

	BLK_FREE(blk);
	_cookie_unset(arena, blk);
	qjoin(arena);
}

static void qjoin(qarena_t *arena) {
	qblock_t *blk = BLK(arena+1);
	qblock_t *end = (qblock_t *)((char *)arena + arena->size);
	
	while (blk && blk < end) {
		qcheck(arena, blk);
		if (BLK_NEXT(blk) < end && BLK_ISFREE(blk) && BLK_ISFREE(BLK_NEXT(blk))) {
			qblock_t *nblk = BLK_NEXT(blk);
			blk->szflag += BLK_SZ(nblk);
#ifdef HEAP_PARANOID
			memset(nblk, 0xAA, sizeof(qblock_t));
#endif
		} else {
			blk = BLK_NEXT(blk);
		}
	}
}

static void qcheck(qarena_t *arena, qblock_t *blk) {
#ifdef HEAP_INTEGRITY
	if (BLK_ISFREE(blk)) {
		if (blk->cookie0 != BLK_COOKIE(arena, blk))
			panic("qcheck: cookie0 corrupt on free blk");
		if (blk->cookie1 != ~BLK_COOKIE(arena, blk))
			panic("qcheck: cookie1 corrupt on free blk");
	} else {
		if (blk->cookie0 != ~BLK_COOKIE(arena, blk))
			panic("qcheck: cookie0 corrupt on alloc blk");
		if (blk->cookie1 != BLK_COOKIE(arena, blk))
			panic("qcheck: cookie1 corrupt on alloc blk");
	}
#endif
#ifdef HEAP_PARANOID
	if (BLK_ISFREE(blk)) {
		unsigned i;
		uint8_t *p = BLK_PAYLOAD(blk);
		
		for (i = 0; i < BLK_SZ(blk) - sizeof(qblock_t); i++)
			if (p[i] != 0xAA) {
				printf("%08x %08x %08x %02x\n", p, i, &p[i], p[i]);
				panic("qcheck: paranoia pays off -- heap corruption deep inside free block");
			}
	}
#endif
}
