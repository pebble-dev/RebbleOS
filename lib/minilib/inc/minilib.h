/* minilib.h
 * Definitions for a very small libc
 * Originally written for NetWatch, a system management mode administration console
 *
 * Authors: Jacob Potter <jacobdp@gmail.com>,
 *          Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 */

#ifndef MINILIB_H
#define MINILIB_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#ifndef NULL
#  define NULL ((void *)0)
#endif

/* minilib.c */
#ifndef _STRING_H_
extern void *memcpy(void *dest, const void *src, int bytes);
extern void *memset(void *dest, int data, int bytes);
extern void *memchr(const void *buf, int c, int maxlen);
extern void *memmove(void *dest, const void *src, int bytes);
extern int memcmp(const char *a2, const char *a1, int bytes);
extern int strcmp(const char *a2, const char *a1);
extern int strncmp(const char *a2, const char *a1, int n);
extern int strlen(const char *c);
extern int strnlen(const char *c, int n);
extern void *strcat(char *dest, const char *src);
extern void *strncat(char *dest, const char *src, int num);
extern void *strcpy(char *a2, const char *a1);
extern char *strncpy(char *a2, const char *a1, size_t len);
extern void tohex(char *s, unsigned long l);
extern void btohex(char *s, unsigned char c);
extern unsigned short htons(unsigned short in);
extern unsigned int htonl(unsigned int in);
extern unsigned short ntohs(unsigned short in);
extern unsigned int ntohl(unsigned int in);
#endif

/* crc32.c */
extern void crc32_init();
extern uint32_t crc32(uint8_t *buf, int len, uint32_t crc0);

/* fmt.c */
struct fmtctx {
	const char *str;
	unsigned int state;
	int num_written;
	unsigned char ungetbuf;
	int (*in)(void *priv, char *c); /* 1 if character available, 0 if EOF */
	void (*out)(void *priv, char c);
	void *priv;
};

int fmt(struct fmtctx *ctx, va_list args);
int vsfmt(char *buf, unsigned int len, const char *ifmt, va_list ap);
int sfmt(char *buf, unsigned int len, const char *ifmt, ...);

/* unfmt.c */
int unfmt(struct fmtctx *ctx, va_list args);
int unvsfmt(const char *buf, const char *ifmt, va_list ap);
int unsfmt(const char *buf, const char *ifmt, ...);

#endif
