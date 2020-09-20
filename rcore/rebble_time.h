#pragma once
/* rebble_time.h
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "FreeRTOS.h"
#include <time.h>

//! Weekday values
typedef enum {
  TODAY = 0,  //!< Today
  SUNDAY,     //!< Sunday
  MONDAY,     //!< Monday
  TUESDAY,    //!< Tuesday
  WEDNESDAY,  //!< Wednesday
  THURSDAY,   //!< Thursday
  FRIDAY,     //!< Friday
  SATURDAY,   //!< Saturday
} WeekDay;


// a bit mask of the time units
typedef enum {
    SECOND_UNIT = 1 << 0, 
    MINUTE_UNIT = 1 << 1, 
    HOUR_UNIT   = 1 << 2, 
    DAY_UNIT    = 1 << 3, 
    MONTH_UNIT  = 1 << 4, 
    YEAR_UNIT   = 1 << 5
} TimeUnits;
typedef void(*TickHandler)(struct tm *tick_time, TimeUnits units_changed);

void rcore_time_init(void);
time_t rcore_mktime(struct tm *tm);
struct tm *rcore_pbl_localtime(time_t *time);

uint16_t rcore_time_ms(time_t *tutc, uint16_t *ms);
TickType_t rcore_time_to_ticks(time_t t, uint16_t ms);

// private
struct tm *rebble_time_get_tm(void);
int pbl_clock_is_24h_style();
uint16_t pbl_time_deprecated(time_t *tloc);
time_t pbl_time_t_deprecated(time_t *tloc);
uint16_t pbl_time_ms_deprecated(time_t *tloc, uint16_t *ms);

time_t clock_to_timestamp(WeekDay day, int hour, int minute);
TickType_t get_boot_tick(void);
time_t rcore_get_time(void);
void rcore_set_tz_name(char *tz_name, uint8_t len);
void rcore_set_time(time_t time);
void rcore_set_utc_offset(uint8_t offset);

/* Time zone routines from tz.c. */

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

struct fd;

void tz_init();
void tz_db_open(struct fd *fd);
int tz_db_nextdir(struct fd *fd, char *name, int nlen);
int tz_db_nexttz(struct fd *fd, char *name, int nlen, union tzrec *tzrec);
int tz_load(const char *dir, const char *name);
time_t tz_utc_to_local(time_t utc, int *dst);
time_t tz_local_to_utc(time_t local, int dst);
const char *tz_name();
