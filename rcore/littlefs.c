#include "stdio.h"
#include "string.h"
#include "platform.h"
#include "stm32_power.h"
#include "log.h"
#include "appmanager.h"
#include "flash.h"
#include "snowy_ext_flash.h"
#include "rebble_memory.h"
#include "lfs.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;
#define OFS 0x408000

int _lfs_wrap_read(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size) 
{
    uint8_t *data = buffer;
    uint32_t address = (cfg->block_size * block) + off;

    /* validity checks */
    assert(off  % cfg->read_size == 0);
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);

    memset(data, 0, size);  
    
    hw_flash_read_bytes(OFS + address, data, size);
    return LFS_ERR_OK;    
}

int _lfs_wrap_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int address = (cfg->block_size * block) + off;
    printf("DANGER W: %d %d %d %x\n", block, cfg->block_count, off, address);
    if (hw_flash_write_bytes(OFS + address, (void *)buffer, size))
        return LFS_ERR_OK;
    return LFS_ERR_IO;
}

int _lfs_wrap_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    int address = cfg->block_size * block;
    if (hw_flash_erase_address(OFS + address))
        return LFS_ERR_OK;

    return LFS_ERR_IO;
}

_lfs_wrap_sync(const struct lfs_config *cfg)
{
    return LFS_ERR_OK;
}

#define LFS_PHYSICAL_BLOCK_COUNT  1
#define LFS_PHYSICAL_PAGESIZE_MAIN 0x8000
#define LFS_PHYSICAL_PAGES_PER_BLOCK 5

#define LFS_PHYSICAL_BLOCK_SIZE          (LFS_PHYSICAL_PAGESIZE_MAIN * LFS_PHYSICAL_PAGES_PER_BLOCK)

#define LFS_LOGICAL_READ_SIZE            32
#define LFS_LOGICAL_PROG_SIZE            32
#define LFS_LOGICAL_BLOCK_SIZE           LFS_PHYSICAL_BLOCK_SIZE
#define LFS_LOGICAL_BLOCK_COUNT          LFS_PHYSICAL_BLOCK_COUNT

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = _lfs_wrap_read,
    .prog  = _lfs_wrap_prog,
    .erase = _lfs_wrap_erase,
    .sync  = _lfs_wrap_sync,

    // block device configuration
    .read_size        = 32, //LFS_LOGICAL_READ_SIZE,
    .prog_size        = 32, //LFS_LOGICAL_PROG_SIZE,
    .block_size       = 0x8000, // 0x8000, <- if using low pages // LFS_LOGICAL_BLOCK_SIZE,
    .block_count      = 6, //LFS_LOGICAL_BLOCK_COUNT,
    .cache_size       = 32,
    .lookahead_size   = 32,
};

void create_config(void)
{
    int a = lfs_file_open(&lfs, &file, "etc/device", LFS_O_RDWR | LFS_O_CREAT);    
    lfs_file_write(&lfs, &file, "Snowy", 5);
    lfs_file_close(&lfs, &file);
}

// entry point
int rbl_littlefs_init(void)
{
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);
    printf("Mounting\n");
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        printf("Failed...  formatting\n");
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
        int sd = 0;
        sd = lfs_mkdir(&lfs, "/var");
        sd = lfs_mkdir(&lfs, "/etc");
        sd = lfs_mkdir(&lfs, "/apps");
        if (!sd)
            printf("MD %d \n", sd);
        
        create_config();
    }

    // read current count
    uint32_t boot_count = 0;
    int a = lfs_file_open(&lfs, &file, "var/boot_count", LFS_O_RDWR | LFS_O_CREAT);
    
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
    lfs_file_close(&lfs, &file);
    
    char dev[6];
    lfs_file_open(&lfs, &file, "etc/device", LFS_O_RDWR);
    lfs_file_read(&lfs, &file, &dev, 5);
    lfs_file_close(&lfs, &file);
    dev[5] = 0;
    printf("Device is: %s\n", dev);
//    lfs_unmount(&lfs);

    printf("boot_count: %d\n", boot_count);
}
