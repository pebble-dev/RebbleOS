/* fs.h
 * definitions for filesystem routines
 * RebbleOS
 */

#pragma once
#include <stdint.h>
#include <stddef.h>

#define FILE_IS_RAW_FLASH 0x1
#define FILE_HAS_DIRENT   0x2 /* i.e., it's not a file-within-a-file */

struct file {
    uint8_t flags;
    uint16_t startpage;
    size_t startpofs;
    uint32_t size;
};

struct fd {
    struct file file;
    struct file replaces;
    
    uint16_t curpage;
    size_t curpofs;
    
    size_t offset;
};

enum seek {
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END
};

void fs_init();
int fs_find_file(struct file *file, const char *name);
void fs_file_from_file(struct file *file, const struct file *from, size_t offset, size_t len);
void fs_file_from_flash(struct file *file, size_t addr, size_t len);
struct fd *fs_creat_replacing(struct fd *fd, const char *name, size_t bytes, const struct file *previous /* can be NULL */);
struct fd *fs_creat(struct fd *fd, const char *name, size_t bytes);
void fs_open(struct fd *fd, const struct file *file);
void fs_mark_written(struct fd *fd);
int fs_read(struct fd *fd, void *p, size_t n);
int fs_write(struct fd *fd, const void *p, size_t n);
long fs_seek(struct fd *fd, long ofs, enum seek whence);
long fs_size(struct fd *fd);
