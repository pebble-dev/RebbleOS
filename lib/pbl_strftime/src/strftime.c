#include "strftime.h"
#include <string.h>

static const char *const s_abbrWeekday[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char *const s_weekday[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};
static const char *const s_abbrMonth[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *const s_month[] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September","October", "November", "December"
};
static const char *const s_12hourSpec[] = {
	"AM", "PM"
};
static const char *const s_prefDateTimeFormat = "%a %b %e %H:%M:%S %Y";
static const char *const s_prefTimeFormat = "%H:%M:%S";
static const char *const s_prefDateFormat = "%m/%d/%y";

static size_t prv_getCharLength(char firstChar) {
	unsigned char firstByte = (unsigned char)firstChar;
	if ((firstByte & 0xE0) == 0xC0) // 110xxxxx
		return 2;
	else if ((firstByte & 0xF0) == 0xE0) // 1110xxxx
		return 3;
	else if ((firstByte & 0xF8) == 0xF0) // 11110xxx
		return 4;
	else
		return 1;
}

static unsigned int prv_uilog10(unsigned int val) {
	static const unsigned int s_pow10[] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
	};
	if (val / s_pow10[1]) {
		if (val / s_pow10[2]) {
			if (val / s_pow10[4]) {
				if (val / s_pow10[8])
					return val / s_pow10[9] ? 9 : 8;
				else {
					if (val / s_pow10[6])
						return val / s_pow10[7] ? 7 : 6;
					else
						return val / s_pow10[5] ? 5 : 4;
				}
			}
			else
				return val / s_pow10[3] ? 3 : 2;
		}
		else
			return 1;
	}
	else
		return 0;
}

static inline int prv_getIsLeapYear(int year) {
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static int prv_getWeekNumISO(const struct tm *timp) {
	// see https://github.com/idunham/musl/blob/master/src/time/strftime.c#L21

	int val = (timp->tm_yday + 7 - (timp->tm_wday + 6) % 7) / 7;
	/* If 1 Jan is just 1-3 days past Monday,
	* the previous week is also in this year. */
	if ((timp->tm_wday - timp->tm_yday - 2 + 371) % 7 <= 2)
		val++;
	if (!val) {
		val = 52;
		/* If 31 December of prev year a Thursday,
		* or Friday of a leap year, then the
		* prev year has 53 weeks. */
		int dec31 = (timp->tm_wday - timp->tm_yday - 1 + 7) % 7;
		if (dec31 == 4 || (dec31 == 5 && prv_getIsLeapYear(timp->tm_year + 1900)))
			val++;
	}
	else if (val == 53) {
		/* If 1 January is not a Thursday, and not
		* a Wednesday of a leap year, then this
		* year has only 52 weeks. */
		int jan1 = (timp->tm_wday - timp->tm_yday + 371) % 7;
		if (jan1 != 4 && (jan1 != 3 || !prv_getIsLeapYear(timp->tm_year + 1900)))
			val = 1;
	}
	return val;
}

static inline int prv_printChar(const char **str, char **bufferPtr, size_t *remSizePtr) {
	size_t length = prv_getCharLength(**str);
	if (length > *remSizePtr)
		return 0;

	while (length) {
		*(*bufferPtr)++ = *(*str)++;
		length--;
		(*remSizePtr)--;
	}
	return 1;
}

static inline int prv_printString(const char *str, char **bufferPtr, size_t *remSizePtr) {
	while (*str && *remSizePtr) {
		if (!prv_printChar(&str, bufferPtr, remSizePtr))
			return 0;
	}
	return *str == '\0';
}

static int prv_printMinPaddedNum(
	unsigned int num,
	size_t length,
	char paddingChar,
	char **bufferPtr,
	size_t *remSizePtr) {

	int overLength = 0;
	if (length > *remSizePtr) {
		overLength = 1;
		while (length > *remSizePtr) {
			num /= 10;
			length--;
		}
		if (length == 0)
			return 0;
	}

	char *writePtr = *bufferPtr;
	*bufferPtr += length;
	do {
		writePtr[--length] = '0' + num % 10;
		num /= 10;
		(*remSizePtr)--;
	} while (length && *remSizePtr && num);

	while (length && *remSizePtr) {
		writePtr[--length] = paddingChar;
		(*remSizePtr)--;
	}

	return length == 0 && !overLength;
}

static inline int prv_printMaxPaddedNum(
	unsigned int num,
	size_t minLength,
	char paddingChar,
	char **bufferPtr,
	size_t *remSizePtr) {

	size_t length = (size_t)prv_uilog10(num) + 1;
	if (length < minLength)
		length = minLength;

	return prv_printMinPaddedNum(num, length, paddingChar, bufferPtr, remSizePtr);
}

static inline int prv_printComposite(
	const char *format,
	const struct tm *timp,
	char **bufferPtr,
	size_t *remSizePtr) {

	size_t outLen;
	size_t result = outLen = FUNC_PBL_STRFTIME(*bufferPtr, *remSizePtr + 1, format, timp); // add one for the terminator
	if (result == 0)
		outLen = strlen(*bufferPtr);
	*bufferPtr += outLen;
	*remSizePtr += outLen;
	return result;
}

size_t FUNC_PBL_STRFTIME(char *buffer, size_t maxSize, const char *format, const struct tm *timp) {
	if (buffer == NULL || maxSize == 0 || format == NULL || timp == NULL)
		return 0;

	int overLength = 0;
	char *const bufferStart = buffer;
	size_t remSize = maxSize - 1; // keep one for the terminator

	while (remSize && *format && !overLength) {
		if (*format == '%') {
			format++;
			switch (*format) {

				// Date numbers
				case('C'): { // century
					if (!prv_printMinPaddedNum((timp->tm_year + 1900) / 100, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('y'): { // last two digits of year
					if (!prv_printMinPaddedNum((timp->tm_year + 1900) % 100, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('Y'): { // full year
					if (!prv_printMaxPaddedNum(timp->tm_year + 1900, 4, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('j'): { // day in the year, one based
					if (!prv_printMinPaddedNum(timp->tm_yday + 1, 3, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('m'): { // month
					if (!prv_printMinPaddedNum(timp->tm_mon + 1, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('d'): { // day of month
					if (!prv_printMinPaddedNum(timp->tm_mday, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('e'): { // day of month, space padding
					if (!prv_printMinPaddedNum(timp->tm_mday, 2, ' ', &buffer, &remSize))
						overLength = 1;
				}break;
				case('u'): { // weekday number, one based from monday
					char wday = (char)timp->tm_wday;
					if (wday == 0)
						wday = 7;
					*(buffer++) = '0' + wday;
					remSize--;
				}break;
				case('w'): { // weekday number, zero based from sunday
					*(buffer++) = '0' + (char)timp->tm_wday;
					remSize--;
				}break;

				// Time numbers
				case('H'): { // hours, 24-hour style, zero padding
					if (!prv_printMinPaddedNum(timp->tm_hour, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('k'): { // hours, 24-hour style, space padding
					if (!prv_printMinPaddedNum(timp->tm_hour, 2, ' ', &buffer, &remSize))
						overLength = 1;
				}break;
				case('I'): { // hours, 12-hour style, zero padding
					int hour = timp->tm_hour % 12;
					if (hour == 0)
						hour = 12;
					if (!prv_printMinPaddedNum(hour, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('l'): { // hours, 12-hour style, space padding
					int hour = timp->tm_hour % 12;
					if (hour == 0)
						hour = 12;
					if (!prv_printMinPaddedNum(hour, 2, ' ', &buffer, &remSize))
						overLength = 1;
				}break;
				case('M'): { // minutes
					if (!prv_printMinPaddedNum(timp->tm_min, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('S'): { // seconds
					if (!prv_printMinPaddedNum(timp->tm_sec, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;

				// Texts
				case('a'): { // weekday name abbreviated
					if (!prv_printString(s_abbrWeekday[timp->tm_wday], &buffer, &remSize))
						overLength = 1;
				}break;
				case('A'): { // weekday name
					if (!prv_printString(s_weekday[timp->tm_wday], &buffer, &remSize))
						overLength = 1;
				}break;
				case('b'):
				case('h'): { // month name abbreviated
					if (!prv_printString(s_abbrMonth[timp->tm_mon], &buffer, &remSize))
						overLength = 1;
				}break;
				case('B'): { // month name
					if (!prv_printString(s_month[timp->tm_mon], &buffer, &remSize))
						overLength = 1;
				}break;
				case('p'): { // 12 hour spec
					if (!prv_printString(s_12hourSpec[timp->tm_hour / 12], &buffer, &remSize))
						overLength = 1;
				}break;
				case('Z'): {
					// TODO: No support for timezones yet
				}break;
				case('%'): {
					*buffer++ = '%';
				}break;
				case('n'): {
					*buffer++ = '\n';
				}break;
				case('t'): {
					*buffer++ = '\t';
				}break;

				// Composites (do not use composite in composite formats please?)
				case('c'): { // preferred date and time
					if (!prv_printComposite(s_prefDateTimeFormat, timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('x'): { // preferred date
					if (!prv_printComposite(s_prefDateFormat, timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('X'): { // preferred time
					if (!prv_printComposite(s_prefTimeFormat, timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('D'): { // date month/day/year
					if (!prv_printComposite("%m/%d/%y", timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('F'): { // ISO 8601:2000 date year-month-day
					if (!prv_printComposite("%Y-%m-%d", timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('r'): { // 12-hour-style time as hours:minutes:seconds timespec
					if (!prv_printComposite("%I:%M:%S %p", timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('R'): { // 24-hour-style time as hours:minutes
					if (!prv_printComposite("%H:%M", timp, &buffer, &remSize))
						overLength = 1;
				}break;
				case('T'): { // 24-hour-style time as hours:minutes:seconds
					if (!prv_printComposite("%H:%M:%S", timp, &buffer, &remSize))
						overLength = 1;
				}break;

				// Week number standards
				case('G'): { // ISO 8601:2000 week-based year
					int year = timp->tm_year + 1900;
					int week = prv_getWeekNumISO(timp);
					if (timp->tm_yday < 3 && week != 1)
						year--;
					else if (timp->tm_yday > 360 && week == 1)
						year++;
					if (!prv_printMaxPaddedNum(year, 4, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('U'): { // week, starts Sundays, 1st contains first Sunday, previous is week 0
					// see https://github.com/idunham/musl/blob/master/src/time/strftime.c#L144
					int week = (timp->tm_yday + 7 - timp->tm_wday) / 7;
					if (!prv_printMinPaddedNum(week, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('V'): { // week, starts Monday, 1st contains January 4th, previous is last year
					int week = prv_getWeekNumISO(timp);
					if (!prv_printMinPaddedNum(week, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;
				case('W'): { // week, starts Monday, 1st contains first Monday, previous is week 0
					// see https://github.com/idunham/musl/blob/master/src/time/strftime.c#L146
					int week = (timp->tm_yday + 7 - (timp->tm_wday + 6) % 7) / 7;
					if (!prv_printMinPaddedNum(week, 2, '0', &buffer, &remSize))
						overLength = 1;
				}break;

				default: { // invalid format
					*buffer++ = '%';
					remSize--;
					if (!prv_printChar(&format, &buffer, &remSize))
						overLength = 1;
					else
						format--; // it gets incremented two lines down again
				}
			}
			format++;
		}
		else {
			if (!prv_printChar(&format, &buffer, &remSize))
				overLength = 1;
		}
	}

	*buffer = '\0'; // we still have one byte by pre-decrementing maxSize
	return *format == '\0' && !overLength ? buffer - bufferStart : 0;
}
