/* sbrk.c
 * Pointer-bumping memory allocator for RebbleOS
 * minilib for RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 *
 * This sbrk() implementation is inherently single-threaded.  Any
 * application using this implementation must provide its own locking
 * mechanism.
 */

#include <stdint.h>
#include <sys/types.h>

/* Linker-defined variables. */
extern char _end;
extern char _ram_top;

static void *heap_end = NULL;

void *sbrk(size_t incr) {
	void *prev_heap_end;
	
	if ((heap_end + incr) >= (void *)&_ram_top)
		return (void *)-1;
	
	prev_heap_end = heap_end;
	heap_end += incr;
	return prev_heap_end;
}
