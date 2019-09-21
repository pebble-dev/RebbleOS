/*
FUNCTION
	<<strcspn>>---count characters not in string

INDEX
	strcspn

ANSI_SYNOPSIS
	size_t strcspn(const char *<[s1]>, const char *<[s2]>);

TRAD_SYNOPSIS
	size_t strcspn(<[s1]>, <[s2]>)
	char *<[s1]>;
	char *<[s2]>;

DESCRIPTION
	This function computes the length of the initial part of
	the string pointed to by <[s1]> which consists entirely of
	characters <[NOT]> from the string pointed to by <[s2]>
	(excluding the terminating null character).

RETURNS
	<<strcspn>> returns the length of the substring found.

PORTABILITY
<<strcspn>> is ANSI C.

<<strcspn>> requires no supporting OS subroutines.
 */

#include "string.h"

size_t strcspn (const char *s1, const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    break;
	}
      if (*c)
	break;
      s1++;
    }

  return s1 - s;
}


/*
FUNCTION
	<<strspn>>---find initial match

INDEX
	strspn

ANSI_SYNOPSIS
	#include <string.h>
	size_t strspn(const char *<[s1]>, const char *<[s2]>);

TRAD_SYNOPSIS
	#include <string.h>
	size_t strspn(<[s1]>, <[s2]>)
	char *<[s1]>;
	char *<[s2]>;

DESCRIPTION
	This function computes the length of the initial segment of
	the string pointed to by <[s1]> which consists entirely of
	characters from the string pointed to by <[s2]> (excluding the
	terminating null character).

RETURNS
	<<strspn>> returns the length of the segment found.

PORTABILITY
<<strspn>> is ANSI C.

<<strspn>> requires no supporting OS subroutines.

QUICKREF
	strspn ansi pure
*/

#include "string.h"

size_t strspn(const char *s1, const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    break;
	}
      if (*c == '\0')
	break;
      s1++;
    }

  return s1 - s;
}


/*
FUNCTION
	<<strchr>>---search for character in string

INDEX
	strchr

ANSI_SYNOPSIS
	#include <string.h>
	char * strchr(const char *<[string]>, int <[c]>);

TRAD_SYNOPSIS
	#include <string.h>
	char * strchr(<[string]>, <[c]>);
	const char *<[string]>;
	int <[c]>;

DESCRIPTION
	This function finds the first occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<strchr>> is ANSI C.

<<strchr>> requires no supporting OS subroutines.

QUICKREF
	strchr ansi pure
*/

#include "string.h"
#include "limits.h"

/* Nonzero if X is not aligned on a "long" boundary.  */
#define UNALIGNED(X) ((long)X & (sizeof (long) - 1))

/* How many bytes are loaded each iteration of the word copy loop.  */
#define LBLOCKSIZE (sizeof (long))

#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

/* DETECTCHAR returns nonzero if (long)X contains the byte used
   to fill (long)MASK. */
#define DETECTCHAR(X,MASK) (DETECTNULL(X ^ MASK))

#ifdef __AVR__
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif /* __AVR__ */

char *strchr(const char *s1, int i)
{
  const unsigned char *s = (const unsigned char *)s1;
  unsigned char c = i;

#if !defined(PREFER_SIZE_OVER_SPEED)
  unsigned long mask,j;
  unsigned long *aligned_addr;

  /* Special case for finding 0.  */
  if (!c)
    {
      while (UNALIGNED (s))
        {
          if (!*s)
            return (char *) s;
          s++;
        }
      /* Operate a word at a time.  */
      aligned_addr = (unsigned long *) s;
      while (!DETECTNULL (*aligned_addr))
        aligned_addr++;
      /* Found the end of string.  */
      s = (const unsigned char *) aligned_addr;
      while (*s)
        s++;
      return (char *) s;
    }

  /* All other bytes.  Align the pointer, then search a long at a time.  */
  while (UNALIGNED (s))
    {
      if (!*s)
        return NULL;
      if (*s == c)
        return (char *) s;
      s++;
    }

  mask = c;
  for (j = 8; j < LBLOCKSIZE * 8; j <<= 1)
    mask = (mask << j) | mask;

  aligned_addr = (unsigned long *) s;
  while (!DETECTNULL (*aligned_addr) && !DETECTCHAR (*aligned_addr, mask))
    aligned_addr++;

  /* The block of bytes currently pointed to by aligned_addr
     contains either a null or the target char, or both.  We
     catch it using the bytewise search.  */

  s = (unsigned char *) aligned_addr;

#endif /* not PREFER_SIZE_OVER_SPEED */

  while (*s && *s != c)
    s++;
  if (*s == c)
    return (char *)s;
  return NULL;
}
