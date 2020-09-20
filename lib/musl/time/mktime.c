#include "time_impl.h"
#include <errno.h>

time_t mktime(struct tm *tm)
{
	struct tm new;
	long opp;
	long long t = __tm_to_secs(tm);
	
	/* XXX: This does not correctly update tm_isdst. */
	t = tz_local_to_utc(t, tm->tm_isdst);

	return t;
}

time_t timegm(struct tm *tm)
{
	struct tm new;
	long opp;
	long long t = __tm_to_secs(tm);

	return t;
}
