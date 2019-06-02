#pragma once
/**
 * @file blob_db_ramfs.h
 * @author Barry Carter
 * @date 8 Jan 2019
 * @brief Blob Database RAM Filesystem. Becuase we can't write to flash yet.
 *
 */

void ramfs_init(void);
void ramfs_open(struct fd *fd, struct file *file, uint8_t database_id);
int ramfs_write(struct fd *fd, void *p, size_t size);
int ramfs_read(struct fd *fd, void *p, size_t bytes);
void ramfs_delete(Uuid *uuid);
long ramfs_seek(struct fd *fd, long ofs, enum seek whence);
