/* unfmt.c
 * Widths hh, h, l, ll
 * Conversions d, x, %
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 * Public domain; optionally see LICENSE
 */

#include <minilib.h>
#include <stdarg.h>

enum {
	ST_CHAR     = 0x00000001, /* half half */
	ST_SHORT    = 0x00000002, /* half */
	ST_LONG     = 0x00000004,
	ST_LONGLONG = 0x00000008,
	ST_WIDTH    = 0x00000010,
	ST_UNGET    = 0x00000020, /* char available in ungetbuf */
};

#define PACKWID(c,w)	((c)->state |= (w << 20))
#define WID(c)		(((c)->state >> 20) & 0xFFF)

static int _in(struct fmtctx *ctx, char *c) {
	int rv;
	
	if (ctx->state & ST_UNGET) {
		*c = ctx->ungetbuf;
		ctx->state &= ~ST_UNGET;
		rv = 1;
	} else
		rv = ctx->in(ctx->priv, c);
	
	return rv;
}

/* must not be called with char already in ungetbuf */
static void _unget(struct fmtctx *ctx, char c) {
	ctx->state |= ST_UNGET;
	ctx->ungetbuf = c;
}

static int _isdigit(char c) {
	return c >= '0' && c <= '9';
}

static int _isspace(char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static unsigned int _atou(const char **p) {
	unsigned int v = 0;
	while (_isdigit(**p)) {
		v *= 10;
		v += (**p - '0');
		(*p)++;
	}
	return v;
}

struct va_list_wrap {
	va_list va;
};

static int _unfmti(struct fmtctx *ctx, char c, struct va_list_wrap *va) {
	unsigned int n = WID(ctx);
	int ishex = (c == 'x');
	int isneg = 0;
	long long val = 0;
	char inc;
	int didconv = 0;
	
	while (!(ctx->state & ST_WIDTH) || n) {
		if (!_in(ctx, &inc))
			break;
		
		if (!didconv && inc == '-') {
			isneg = 1;
			continue;
		}
		
		if (ishex) {
			if (_isdigit(inc)) {
				val *= 16;
				val += inc - '0';
			} else if (inc >= 'a' && inc <= 'f') {
				val *= 16;
				val += inc - 'a' + 0xa;
			} else if (inc >= 'A' && inc <= 'F') {
				val *= 16;
				val += inc - 'A' + 0xa;
			} else {
				_unget(ctx, inc);
				break;
			}
		} else {
			if (!_isdigit(inc)) {
				_unget(ctx, inc);
				break;
			}
			
			val *= 10;
			val += inc - '0';
		}
		
		didconv = 1;
		n--;
	}
	
	if (didconv) {
		if (isneg)
			val = -val;
		if (ctx->state & ST_CHAR)
			*va_arg(va->va, char *) = (char)val;
		else if (ctx->state & ST_SHORT)
			*va_arg(va->va, short *) = (short)val;
		else if (ctx->state & ST_LONG)
			*va_arg(va->va, long *) = (long)val;
		else if (ctx->state & ST_LONGLONG)
			*va_arg(va->va, long long *) = val;
		else
			*va_arg(va->va, int *) = (int)val;
	}
	
	return didconv;
}

static int _unfmtpct(struct fmtctx *ctx, char c, struct va_list_wrap *va) {
	(void)c;
	(void)va;
	
	char inc;
	
	if (!_in(ctx, &inc))
		return 0;
	
	if (inc != '%')
		return 0;
	
	return 1;
}

static struct {
	char f;
	int (*func)(struct fmtctx *ctx, char c, struct va_list_wrap *va);
} unfmts[] = {
	{ 'd', _unfmti },
	{ 'x', _unfmti },
	{ '%', _unfmtpct },
	{ '\0', 0 }
};

int unfmt(struct fmtctx *ctx, va_list args) {
	int i, convs = 0;
	ctx->state = 0;
	
	struct va_list_wrap wrap;
	va_copy(wrap.va, args);
	
	while (*ctx->str) {
		if (_isspace(*ctx->str)) {
			char c;
			
			do {
				if (!_in(ctx, &c))
					return convs;
			} while (_isspace(c));
			
			_unget(ctx, c);
			
			ctx->str++;
			
			continue;
		}
		
		if (*ctx->str != '%') {
			char c;
			
			if (!_in(ctx, &c))
				return convs;
			
			if (c != *ctx->str)
				return convs;
			
			ctx->str++;
			
			continue;
		}

		ctx->str++;
		ctx->state &= ST_UNGET;

		if (_isdigit(*ctx->str)) {
			PACKWID(ctx, _atou(&ctx->str));
			ctx->state |= ST_WIDTH;
		}

		if (*ctx->str == 'l') {
			ctx->str++;
			ctx->state |= (*ctx->str == 'l' ? ST_LONGLONG : ST_LONG);
		}

		if (*ctx->str == 'h') {
			ctx->str++;
			ctx->state |= (*ctx->str == 'h' ? ST_CHAR : ST_SHORT);
		}
		
		int didconv = 0;
		for (i = 0; unfmts[i].f; i++)
			if (unfmts[i].f == *ctx->str)
				didconv = unfmts[i].func(ctx, *ctx->str, &wrap);
		
		convs += didconv;
		if (!didconv)
			return convs;

		ctx->str++;
	}
	return convs;
}

struct sunfmtctx {
	const char *buf;
	unsigned int idx;
};

static int _sunfmtin(void *p, char *c) {
	struct sunfmtctx *ctx = p;
	if (!ctx->buf[ctx->idx])
		return 0;
	*c = ctx->buf[ctx->idx++];
	return 1;
}

int unvsfmt(const char *buf, const char *ifmt, va_list ap) {
	struct fmtctx ctx;
	struct sunfmtctx sctx;

	ctx.str = ifmt;
	ctx.in = _sunfmtin;
	ctx.priv = &sctx;

	sctx.buf = buf;
	sctx.idx = 0;

	return unfmt(&ctx, ap);
}

int unsfmt(const char *buf, const char *ifmt, ...) {
	va_list ap;
	int n;
	
	va_start(ap, ifmt);
	n = unvsfmt(buf, ifmt, ap);
	va_end(ap);
	
	return n;
}

#ifdef TEST
#include <stdio.h>

int main() {
	int convs[4];
	int rv;
	
	rv = unsfmt("hmm...?\t\t\nab0:cD:2345% \t\r934x5", "hmm...? %2x0:%2x:%d%% %2d", convs, convs+1, convs+2, convs+3);
	
	printf("%d: %x %x %d %d", rv, convs[0], convs[1], convs[2], convs[3]);
	
	return 0;
}
#endif
