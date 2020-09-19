/* tz.c
 * Time-zone database loading and conversion routnies
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "minilib.h"

#include "resource.h"
#include "fs.h"
#include "log.h"
#include "rebble_time.h"

#include "platform_res.h"

#define MODULE_NAME "rtime"
#define MODULE_TYPE "KERN"
#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG //RBL_LOG_LEVEL_ERROR

static union tzrec _curtz;

void tz_init() {
	/* Default to GMT. */
	_curtz.hasdst = 0;
	_curtz.nodst.offset = 0;
	
	tz_load("America", "Los_Angeles");
}

void tz_db_open(struct fd *fd) {
	struct file tzfile;
	
	resource_file(&tzfile, resource_get_handle_system(RESOURCE_ID_TZDB));
	fs_open(fd, &tzfile);
}

int tz_db_nextdir(struct fd *fd, char *name, int nlen) {
	while (1) {
		uint8_t ty;
		if (fs_read(fd, &ty, 1) < 1)
			return -1;
		if (ty == 1) {
			if (fs_read(fd, &ty, 1) < 1) {
				LOG_ERROR("read(ty == 1, len)");
				return -1;
			}
			if (ty > nlen) {
				fs_seek(fd, ty, FS_SEEK_CUR);
				continue;
			}
			
			if (fs_read(fd, name, ty) < ty) {
				LOG_ERROR("read(ty == 1, data)");
				return -1;
			}
			
			return 0;
		} else if (ty == 2) {
			if (fs_read(fd, &ty, 1) < 1) {
				LOG_ERROR("read(ty == 2, len)");
				return -1;
			}
			fs_seek(fd, ty, FS_SEEK_CUR);
			
			union tzrec tzrec;
			if (fs_read(fd, &tzrec, 1) < 1) {
				LOG_ERROR("read(ty == 2, tzrec[0])");
				return -1;
			}
			if (tzrec.hasdst) {
				fs_seek(fd, sizeof(tzrec.dst) - 1, FS_SEEK_CUR);
			} else {
				fs_seek(fd, sizeof(tzrec.nodst) - 1, FS_SEEK_CUR);
			}
		} else {
			LOG_ERROR("invalid ty %d\n", ty);
			return -1;
		}
	}
	
	return -1;	
}

int tz_db_nexttz(struct fd *fd, char *name, int nlen, union tzrec *tzrec) {
	while (1) {
		uint8_t ty;
		
		if (fs_read(fd, &ty, 1) < 1)
			return -1;
		if (ty == 1) {
			fs_seek(fd, -1, FS_SEEK_CUR);
			return -1;
		} else if (ty == 2) {
			if (fs_read(fd, &ty, 1) < 1) {
				LOG_ERROR("read(ty == 1, len)");
				return -1;
			}
			
			int toobig = 0;
			if (ty > nlen) {
				fs_seek(fd, ty, FS_SEEK_CUR);
				toobig = 1;
			} else if (fs_read(fd, name, ty) < ty) {
				LOG_ERROR("read(ty == 1, data)");
				return -1;
			}
			
			if (fs_read(fd, tzrec, 1) < 1) {
				LOG_ERROR("read(ty == 2, tzrec[0])");
				return 1;
			}
			if (tzrec->hasdst) {
				if (fs_read(fd, &tzrec->dst.offset, sizeof(tzrec->dst) - 1) < (sizeof(tzrec->dst) - 1)) {
					LOG_ERROR("read(ty == 2, hasdst, tzrec)");
					return -1;
				}
			} else {
				if (fs_read(fd, &tzrec->nodst.offset, sizeof(tzrec->nodst) - 1) < (sizeof(tzrec->nodst) - 1)) {
					LOG_ERROR("read(ty == 2, nodst, tzrec)");
					return -1;
				}
			}
			
			if (!toobig)
				return 0;
		} else {
			LOG_ERROR("invalid ty %d\n", ty);
			return -1;
		}
	}
}

int tz_load(const char *dir, const char *name) {
	struct fd fd;
	
	tz_db_open(&fd);	
	
	union tzrec tzrec;
	char nambuf[64] = "";
	int rv = 0;
	int isdirnam = 0;
	
	while ((rv = tz_db_nextdir(&fd, nambuf, sizeof(nambuf))) == 0)
		if (strcmp(nambuf, dir) == 0)
			break;
	if (rv) {
		LOG_ERROR("did not find tzdir with name %s/%s", dir, name);
		return -1;
	}
	
	while ((rv = tz_db_nexttz(&fd, nambuf, sizeof(nambuf), &tzrec)) == 0)
		if (strcmp(nambuf, name) == 0)
			break;
	if (rv) {
		LOG_ERROR("did not find tz with name %s/%s", dir, name);
		return -1;
	}
	
	LOG_DEBUG("found tz %s/%s", dir, name);
	memcpy(&_curtz, &tzrec, sizeof(_curtz));
	return 0;
}

/*
** Given a year and a rule, calculate the offset from the start of the year
** in which the rule takes effect.  (Adapted from transtime in localtime.c,
** from the tz distribution.)
*/

#define MONSPERYEAR   12
#define HOURSPERDAY   24
#define SECSPERMIN    60
#define MINSPERHOUR   60
#define SECSPERHOUR   (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY    ((int32_t) SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK   7
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

static const int	mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int32_t
_transtime(const int year, int mode, uint16_t *params)
{
	int	leapyear;
	int32_t value;
	int	i;
	int	d, m1, yy0, yy1, yy2, dow;

	leapyear = isleap(year);
	switch (mode) {
	case 0 /* MODE_JULIAN */:
		/*
		** Jn - Julian day, 1 == January 1, 60 == March 1 even in leap
		** years.
		** In non-leap years, or if the day number is 59 or less, just
		** add SECSPERDAY times the day number-1 to the time of
		** January 1, midnight, to get the day.
		*/
		value = (params[0] - 1) * SECSPERDAY;
		if (leapyear && params[0] >= 60)
			value += SECSPERDAY;
		break;

	case 1 /* MODE_JULIANZ */:
		/*
		** n - day of year.
		** Just add SECSPERDAY times the day number to the time of
		** January 1, midnight, to get the day.
		*/
		value = params[0] * SECSPERDAY;
		break;

	case 2 /* MONTH_NTH_DAY_OF_WEEK */:
		/*
		** Mm.n.d - nth "dth day" of month m.
		*/

		/*
		** Use Zeller's Congruence to get day-of-week of first day of
		** month.
		*/
		m1 = (params[0] /* mon */ + 9) % 12 + 1;
		yy0 = (params[0] /* mon */ <= 2) ? (year - 1) : year;
		yy1 = yy0 / 100;
		yy2 = yy0 % 100;
		dow = ((26 * m1 - 2) / 10 +
			1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
		if (dow < 0)
			dow += DAYSPERWEEK;

		/*
		** "dow" is the day-of-week of the first day of the month. Get
		** the day-of-month (zero-origin) of the first "dow" day of the
		** month.
		*/
		d = params[2] /* day */ - dow;
		if (d < 0)
			d += DAYSPERWEEK;
		for (i = 1; i < params[1] /* week */; ++i) {
			if (d + DAYSPERWEEK >=
				mon_lengths[leapyear][params[0] /* mon */ - 1])
					break;
			d += DAYSPERWEEK;
		}

		/*
		** "d" is the day-of-month (zero-origin) of the day we want.
		*/
		value = d * SECSPERDAY;
		for (i = 0; i < params[0] /* mon */ - 1; ++i)
			value += mon_lengths[leapyear][i] * SECSPERDAY;
		break;
	}

	/*
	** "value" is the year-relative time of 00:00:00 UT on the day in
	** question. To get the year-relative time of the specified local
	** time on that day, add the transition time and the current offset
	** from UT.
	*/
	return value;
}

time_t tz_utc_to_local(time_t utc, int *dst) {
	if (!_curtz.hasdst) {
		*dst = 0;
		return utc + _curtz.nodst.offset;
	}
	
	/* Figure out whether the time_t is within DST.  Start off by
	 * figuring out what year it is, and when that year started...
	 */
	struct tm tm;
	gmtime_r(&utc, &tm);
	
	int year = tm.tm_year + 1900;
	
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = 1;
	tm.tm_mon = 0;
	extern time_t timegm(struct tm *tm);
	time_t tt_yearstart = timegm(&tm); /* a GNU extension, but we implement it in RebbleOS, too */
	
	/* Then figure out when DST starts and ends in that year... */
	time_t tt_dststart  = tt_yearstart + _transtime(year, _curtz.dst.dst_start_mode, _curtz.dst.dst_start_param) + _curtz.dst.dst_start_time + _curtz.dst.   offset;
	time_t tt_dstend    = tt_yearstart + _transtime(year, _curtz.dst.dst_end_mode  , _curtz.dst.dst_end_param  ) + _curtz.dst.dst_end_time   + _curtz.dst.dstoffset;
	
	*dst = utc >= tt_dststart && utc < tt_dstend;
	
	if (*dst)
		return utc - _curtz.dst.dstoffset;
	else
		return utc - _curtz.dst.   offset;
}

time_t tz_local_to_utc(time_t local, int dst) {
	if (!_curtz.hasdst)
		return local + _curtz.nodst.offset;
	
	if (dst)
		return local + _curtz.dst.dstoffset;
	else
		return local + _curtz.dst.offset;
	/* Well, that was easy. */
}
