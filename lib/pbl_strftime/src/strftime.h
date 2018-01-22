#ifndef STRFTIME_H
#define STRFTIME_H

#include <time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Rename function to prevent confusion with standard strftime
#ifdef NOT_REBBLE_OS
#define FUNC_PBL_STRFTIME pbl_strftime
#else
#define FUNC_PBL_STRFTIME strftime
#endif

size_t FUNC_PBL_STRFTIME(char* buffer, size_t maxSize, const char* format, const struct tm* timp);

#ifdef __cplusplus
}
#endif

#endif // STRFTIME_H
