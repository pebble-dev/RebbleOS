/* minilib.c
 * General-purpose C library routines
 * Originally written for NetWatch, a system management mode administration console
 *
 * Authors: Jacob Potter <jacobdp@gmail.com>,
 *          Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 */

#include "minilib.h"

/* We have both _memcpy and memcpy, because gcc might be able to do better in lwip.
 * For small things, gcc inlines its memcpy, but for large things, we call out
 * to this memcpy.
 */
void _memcpy_fast(void *dest, const void *src, int bytes)
{
	/* I hate everyone */
	/* Since we otherwise compile with -O0, we might as well manually speed this up a bit. */
	
	char *cdest = dest;
	const char *csrc = src;
	int *idest;
	const int *isrc;
	int nwords;
	
	/* Align to src (picked arbitrarily; might as well align to something) */
	while (bytes && ((unsigned int)csrc & 3))
	{
		*(cdest++) = *(csrc++);
		bytes--;
	}
	
	idest = (int *)cdest;
	isrc = (const int *)csrc;
	
	nwords = bytes / 4;
	bytes -= bytes & ~3;
	if (nwords != 0)
		switch(nwords % 8)	/* They see me Duff'sin'.  They hatin'. */
			do {
		case  0:	nwords--; *(idest++) = *(isrc++);
		case  7:	nwords--; *(idest++) = *(isrc++);
		case  6:	nwords--; *(idest++) = *(isrc++);
		case  5:	nwords--; *(idest++) = *(isrc++);
		case  4:	nwords--; *(idest++) = *(isrc++);
		case  3:	nwords--; *(idest++) = *(isrc++);
		case  2:	nwords--; *(idest++) = *(isrc++);
		case  1:	nwords--; *(idest++) = *(isrc++);
			} while (nwords);
	
	cdest = (char *)idest;
	csrc = (const char *)isrc;
	while (bytes)	/* Clean up the remainder */
	{
		*(cdest++) = *(csrc++);
		bytes--;
	}
}

void _memcpy_slow(void *dest, const void *src, int bytes)
{
	unsigned char *cdest = dest;
	const unsigned char *csrc = src;
	
	while (bytes--)
		*(cdest++) = *(csrc++);
}

void *memcpy(void *dest, const void *src, int bytes)
{
	if (((unsigned int)dest & 3) == ((unsigned int)src & 3))
		_memcpy_fast(dest, src, bytes);
	else
		_memcpy_slow(dest, src, bytes);
	return dest;
}

void *memset(void *dest, int data, int bytes)
{
	unsigned char *cdest = dest;
	while (bytes--)
		*(cdest++) = (unsigned char)data;
	return dest;
}

void *memchr(const void *buf, int c, int maxlen)
{
	const unsigned char * cbuf = buf;
	while (maxlen--)
	{
		if (*cbuf == c) return (void *)cbuf;
		cbuf++;
	}
	return 0;
}

void *memmove(void *dest, const void *src, int bytes)
{
	char *cdest = dest;
	const char *csrc = src;
	if ((cdest > csrc) && (cdest <= (csrc + bytes)))
	{
		/* do it backwards! */
		cdest += bytes;
		csrc += bytes;
		while (bytes--)
			*(--cdest) = *(--csrc);
	} else
		memcpy(dest, src, bytes);
	
	return dest;
}

int memcmp (const char *a2, const char *a1, int bytes) {
	while (bytes--)
	{
		if (*(a2++) != *(a1++))
			return 1;
	}
	return 0;
}

int strcmp (const char *a2, const char *a1) {
	while (1) {
		if (*a2 != *a1) return 1;
		if (*a2 == 0) return 0;
		a1++;
		a2++;
	}
}

int strncmp (const char *a2, const char *a1, int n) {
	while (n--) {
		if (*a2 != *a1) return 1;
		if (*a2 == 0) return 0;
		a1++;
		a2++;
	}
	return 0;
}

int strlen(const char *c)
{
	int l = 0;
	while (*(c++))
		l++;
	return l;
}

void *strcpy(char *a2, const char *a1)
{
	char *origa2 = a2;
	
	do {
		*(a2++) = *a1;
	} while (*(a1++));
	
	return origa2;
}

void *strcat(char *dest, const char *src)
{
	char *origdest = dest;
	
	while (*dest)
		dest++;
	while (*src)
		*(dest++) = *(src++);
	*(dest++) = *(src++);
	
	return origdest;
}
static char hexarr[] = "0123456789ABCDEF";
void tohex(char *s, unsigned long l)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		s[i] = hexarr[l >> 28];
		l <<= 4;
	}
}

void btohex(char *s, unsigned char c)
{
	s[0] = hexarr[c >> 4];
	s[1] = hexarr[c & 0xF];
}

unsigned short htons(unsigned short in)
{
	return (in >> 8) | (in << 8);
}

unsigned int htonl(unsigned int in)
{
	return ((in & 0xff) << 24) |
	       ((in & 0xff00) << 8) |
	       ((in & 0xff0000UL) >> 8) |
	       ((in & 0xff000000UL) >> 24);
}

int atoi(const char *c)
{
	if (!c) return 0;
	/* Drop leading whitespace */
	while (*c == ' ' || *c == '\t') c++;

	/* Handle sign */
	int sign = 1;
	if (*c == '-') { sign = -1; c++; }
	else if (*c == '+') c++;

	int res = 0;
	while (*c >= '0' && *c <= '9')
		res = res * 10 + (*c++ - '0');

	return res * sign;
}
