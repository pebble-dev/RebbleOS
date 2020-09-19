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
