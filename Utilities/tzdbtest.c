#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

union tzrec {
	uint8_t hasdst;
	struct {
		uint8_t hasdst;
		int32_t offset;
	} __attribute__((packed)) nodst;
	struct {
		uint8_t hasdst;
		int32_t offset;
		int32_t dstoffset;
		uint8_t dst_start_mode;
		uint16_t dst_start_param[3];
		uint32_t dst_start_time;
		uint8_t dst_end_mode;
		uint16_t dst_end_param[3];
		uint32_t dst_end_time;
	} __attribute__((packed)) dst;
};

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
transtime(const int year, int mode, uint16_t *params)
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

time_t utc_to_local(time_t utc, int *dst, union tzrec *tzrec) {
	if (!tzrec->hasdst) {
		*dst = 0;
		return utc + tzrec->nodst.offset;
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
	time_t tt_yearstart = timegm(&tm); /* a GNU extension, but we'll implement it in RebbleOS, too */
	
	/* Then figure out when DST starts and ends in that year... */
	time_t tt_dststart  = tt_yearstart + transtime(year, tzrec->dst.dst_start_mode, tzrec->dst.dst_start_param) + tzrec->dst.dst_start_time + tzrec->dst.   offset;
	time_t tt_dstend    = tt_yearstart + transtime(year, tzrec->dst.dst_end_mode  , tzrec->dst.dst_end_param  ) + tzrec->dst.dst_end_time   + tzrec->dst.dstoffset;
	
	*dst = utc >= tt_dststart && utc < tt_dstend;
	
	if (*dst)
		return utc - tzrec->dst.dstoffset;
	else
		return utc - tzrec->dst.   offset;
}

time_t local_to_utc(time_t local, int dst, union tzrec *tzrec) {
	if (!tzrec->hasdst)
		return local + tzrec->nodst.offset;
	
	if (dst)
		return local + tzrec->dst.dstoffset;
	else
		return local + tzrec->dst.offset;
	/* Well, that was easy. */
}

void do_tz_tests(union tzrec *tzrec) {
	struct tm tm;
	time_t tt = time(NULL);
	localtime_r(&tt, &tm);
	
	int year = tm.tm_year + 1900;
	
	printf("now is %ld, year %d\n", tt, tm.tm_year + 1900);
	
	struct tm tm_yearstart = tm;
	tm_yearstart.tm_sec = 0;
	tm_yearstart.tm_min = 0;
	tm_yearstart.tm_hour = 0;
	tm_yearstart.tm_mday = 1;
	tm_yearstart.tm_mon = 0;
	tm_yearstart.tm_isdst = 0;
	time_t tt_yearstart = timegm(&tm_yearstart);
	
	if (tzrec->hasdst) {
		printf("has dst\n");
		time_t startofs = transtime(year, tzrec->dst.dst_start_mode, tzrec->dst.dst_start_param) + tzrec->dst.dst_start_time + tzrec->dst.offset;
		time_t endofs = transtime(year, tzrec->dst.dst_end_mode, tzrec->dst.dst_end_param) + tzrec->dst.dst_end_time + tzrec->dst.dstoffset;
		
		time_t tt_start = tt_yearstart + startofs;
		time_t tt_end   = tt_yearstart + endofs;
		
		struct tm tm_start;
		struct tm tm_end;
		localtime_r(&tt_start, &tm_start);
		localtime_r(&tt_end,   &tm_end);
		
		printf("startofs: %ld: %s", startofs, asctime(&tm_start));
		printf("endofs:   %ld: %s", endofs,   asctime(&tm_end  ));
		
		time_t before_dst       = tt_start - 1;
		time_t in_dst_barely    = tt_start + 1;
		time_t in_dst_fully     = tt_start + 2 * 60 * 60;
		time_t in_dst_pre_end   = tt_end   - 1;
		time_t after_dst_barely = tt_end   + 1;
		time_t after_dst        = tt_end   + 2 * 60 * 60;
		
		struct tests {
			const char *name;
			time_t tt;
		};
		
		struct tests tests[] = {
			{ "Before DST",             tt_start - 1 },
			{ "In DST, barely",         tt_start     },
			{ "In DST, by a long shot", tt_start + 2 * 60 * 60 },
			{ "Just before DST exit",   tt_end - 1 },
			{ "After DST, barely",      tt_end },
			{ "After DST, solidly",     tt_end + 2 * 60 * 60 },
			{ NULL, 0 }
		};
		
		for (struct tests *test = tests; test->name; test++) {
			int dst;
			time_t local;
			
			local = utc_to_local(test->tt, &dst, tzrec);
			printf("%s (computed dst %d): \n  Sys: %s", test->name, dst, asctime(localtime(&test->tt)));
			printf("  Rbl: %s", asctime(gmtime(&local)));
		}
	}
}

int main(int argc, char **argv) {
	union tzrec tzrec;
	
	if (argc < 3) {
		printf("usage: %s file category tz\n", argv[0]);
		return 1;
	}
	
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open(argv[1])");
		return 1;
	}
	int isdirnam = 0;
	char nambuf[256];
	while (1) {
		uint8_t ty;
		if (read(fd, &ty, 1) < 1)
			return 2;
		if (ty == 1) {
			if (read(fd, &ty, 1) < 1) {
				perror("read(ty == 1, len)");
				return 1;
			}
			if (read(fd, nambuf, ty) < ty) {
				perror("read(ty == 1, data)");
				return 1;
			}
			isdirnam = !strcmp(argv[2], nambuf);
		} else if (ty == 2) {
			if (read(fd, &ty, 1) < 1) {
				perror("read(ty == 2, len)");
				return 1;
			}
			if (read(fd, nambuf, ty) < ty) {
				perror("read(ty == 2, name)");
				return 1;
			}
			
			int isnam = isdirnam && !strcmp(argv[3], nambuf);
			
			if (read(fd, &tzrec, 1) < 1) {
				perror("read(ty == 2, tzrec[0])");
				return 1;
			}
			if (tzrec.hasdst) {
				if (read(fd, &tzrec.dst.offset, sizeof(tzrec.dst) - 1) < (sizeof(tzrec.dst) - 1)) {
					perror("read(ty == 2, hasdst, tzrec)");
					return 1;
				}
			} else {
				if (read(fd, &tzrec.nodst.offset, sizeof(tzrec.nodst) - 1) < (sizeof(tzrec.nodst) - 1)) {
					perror("read(ty == 2, nodst, tzrec)");
					return 1;
				}
			}
			
			if (isnam) {
				printf("... %s %s ...\n", argv[2], argv[3]);
				do_tz_tests(&tzrec);
				return 0;
			}
		} else {
			printf("invalid ty %d\n", ty);
			return 1;
		}
	}
	
	printf("tz not found\n");
	return 2;
}
