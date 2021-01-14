#include "time_impl.h"

struct tm *__localtime_r(const time_t *restrict, struct tm *restrict);

static struct tm _tm;

struct tm *localtime(const time_t *t)
{
	return localtime_r(t, &_tm);
}

struct tm *gmtime(const time_t *t)
{
	static struct tm tm;
	return gmtime_r(t, &_tm);
}
