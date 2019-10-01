/* blob_db_ramfs.c
 * A simple Ram Filesystem for blob database.
 * XXX TODO when we can write flash, remove this
 * RebbleOS
 * 
 * Author: Barry Carter <barry.carter@gmail.com>.
 */
#include "rebbleos.h"
#include "node_list.h"
#include "blob_db.h"

#define MODULE_NAME "blobfs"
#define MODULE_TYPE "SYS"
#define LOG_LEVEL RBL_LOG_LEVEL_ERROR //RBL_LOG_LEVEL_ERROR


/* RAM FS */
#define MSG_HEAP_SIZE 10000

static uint8_t _ramfs_heap[MSG_HEAP_SIZE] MEM_REGION_RAMFS;
static qarena_t *_ramfs_arena;

static list_head _ramfs_head = LIST_HEAD(_ramfs_head);

typedef struct ramfs_t {
    uint8_t database_id;
    uint8_t *data;
    size_t data_size;
    list_node node;
} ramfs;

void ramfs_init(void)
{
    _ramfs_arena = qinit(_ramfs_heap, MSG_HEAP_SIZE);
}

static void *ramfs_calloc(size_t count, size_t size)
{
    /* uses a special qarena */
    void *x = qalloc(_ramfs_arena, count * size);
    if (x != NULL)
        memset(x, 0, count * size);
    return x;
}

static void ramfs_free(void *mem)
{
    qfree(_ramfs_arena, mem);
}

static void *ramfs_realloc(void *mem, size_t size)
{
    return qrealloc(_ramfs_arena, mem, size);
}

static ramfs *_get_db(uint8_t database_id)
{
    ramfs *fs = NULL;
    uint8_t found = 0;

    list_foreach(fs, &_ramfs_head, ramfs, node)
    {
        if (fs->database_id == database_id)
            return fs;
    }

    return NULL;
}

void ramfs_open(struct fd *fd, struct file *file, uint8_t database_id)
{
    ramfs *fs = _get_db(database_id);

    if (!fs)
    {
        fs = ramfs_calloc(1, sizeof(ramfs));
    }

    file->size = fs->data_size;
    file->startpage = database_id;
    file->is_ramfs = 1;
    fs_open(fd, file);

    LOG_DEBUG("RAMFS OPEN: %d,", fd->file.size);
}

int ramfs_write(struct fd *fd, void *p, size_t size)
{
    ramfs *fs = _get_db(fd->curpage);

    if (!fs)
    {
        fs = ramfs_calloc(1, sizeof(ramfs));
        assert(fs);
    }

    if (!fs->data)
    {
        fs->data = ramfs_calloc(1, size);
    }
    else
    {
        fs->data = ramfs_realloc(fs->data, fs->data_size + size);
    }
    LOG_DEBUG("RAMFS INSERT: %d, %d, %x", fs->data_size, size, fs->data);
    assert(fs->data);

    memcpy(fs->data + fs->data_size, p, size);
    fs->database_id = fd->curpage;
    fs->data_size += size;
    fd->curpofs += size;
    fd->offset += size;

    if (fs->node.next == NULL && fs->node.prev == NULL)
    {
        list_init_node(&fs->node);
        list_insert_head(&_ramfs_head, &fs->node);
    }

    return size;
}

int ramfs_read(struct fd *fd, void *p, size_t size)
{
    ramfs *fs = _get_db(fd->curpage);

    if (size > (fd->file.size - fd->offset))
        size = fd->file.size - fd->offset;

    memcpy(p, fs->data + fd->curpofs, size); 
    fd->curpofs += size;
    fd->offset += size;

    return size;
}

long ramfs_seek(struct fd *fd, long ofs, enum seek whence)
{
    size_t newoffset;
    switch (whence)
    {
        case FS_SEEK_SET: newoffset = ofs; break;
        case FS_SEEK_CUR: newoffset = fd->offset + ofs /* XXX: overflow */; break;
        case FS_SEEK_END: newoffset = fd->file.size + ofs; break;
    }

    if (newoffset > fd->file.size)
        newoffset = fd->file.size;

    fd->curpofs = newoffset;
    fd->offset = newoffset;
    LOG_DEBUG("RAMFS SEEK: %d %d,", fd->curpofs, newoffset);
    return fd->offset;
}

void ramfs_delete(Uuid *uuid)
{
//     ramfs *fs;
//     list_foreach(fs, &_ramfs_head, ramfs, node)
//     {
//         if (uuid_equal(&fs->uuid, uuid))
//         {
//             list_remove(&_ramfs_head, &fs->node);
//             ramfs_free(fs->data);
//             ramfs_free(fs);
//             return;
//         }
//     }
}
