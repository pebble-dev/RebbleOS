#include "time_impl.h"
#include <errno.h>
#include <limits.h>
//#include "libc.h"

struct tm *localtime_r(const time_t *restrict t, struct tm *restrict tm)
{
	/* Reject time_t values whose year would overflow int because
	 * __secs_to_zone cannot safely handle them. */
	if (*t < INT_MIN * 31622400LL || *t > INT_MAX * 31622400LL) {
		/* errno = EOVERFLOW; */
		return 0;
	}
	/* __secs_to_zone(*t, 0, &tm->tm_isdst, &tm->__tm_gmtoff, 0, &tm->__tm_zone); */
	if (__secs_to_tm((long long)*t /* + tm->__tm_gmtoff */, tm) < 0) {
		/* errno = EOVERFLOW; */
		return 0;
	}
	return tm;
}

struct tm *gmtime_r(const time_t *restrict t, struct tm *restrict tm)
{
	/* Reject time_t values whose year would overflow int because
	 * __secs_to_zone cannot safely handle them. */
	if (*t < INT_MIN * 31622400LL || *t > INT_MAX * 31622400LL) {
		/* errno = EOVERFLOW; */
		return 0;
	}

	if (__secs_to_tm((long long)*t, tm) < 0) {
		/* errno = EOVERFLOW; */
		return 0;
	}
	return tm;
}
