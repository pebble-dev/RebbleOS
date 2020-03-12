/* fs.h
 * definitions for filesystem routines
 * RebbleOS
 */

#pragma once
#include <stdint.h>
#include <stddef.h>

struct file {
    uint16_t startpage;
    size_t startpofs;
    uint32_t size;
};

struct fd {
    struct file file;
    struct file replaces;
    
    uint16_t curpage;
    uint16_t curpofs;
    
    size_t offset;
};

enum seek {
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END
};

void fs_init();
int fs_find_file(struct file *file, const char *name);
struct fd *fs_creat_replacing(struct fd *fd, const char *name, size_t bytes, const struct file *previous /* can be NULL */);
struct fd *fs_creat(struct fd *fd, const char *name, size_t bytes);
void fs_open(struct fd *fd, const struct file *file);
void fs_mark_written(struct fd *fd);
int fs_read(struct fd *fd, void *p, size_t n);
int fs_write(struct fd *fd, const void *p, size_t n);
long fs_seek(struct fd *fd, long ofs, enum seek whence);
long fs_size(struct fd *fd);

