/* fs_internal.h
 * declarations of structures in filesystem innards, not to be touched by end users
 * RebbleOS
 */

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "fs.h"

struct fs_page_hdr {
    uint16_t v_0x5001;
    uint8_t  empty; /* 0xFF if empty, 0xFC if not empty, 0xFE if the first empty block before the rest of the device is empty? */
#define HDR_EMPTY_ALLOCATED 0x1
#define HDR_EMPTY_MOREBLOCKS 0x2
    uint8_t  status;   /* bitfield; f4, f8, ... */
#define HDR_STATUS_VALID 0x1
#define HDR_STATUS_DEAD 0x2
#define HDR_STATUS_FILE_START 0x4
#define HDR_STATUS_FILE_CONT 0x8
    uint32_t rsvd_0; /* ff ff ff ff */
    uint32_t wear_level_counter;
    uint32_t rsvd_1;
    uint32_t rsvd_2;
    uint8_t  rsvd_3;
    uint8_t  next_page_crc;
    uint16_t next_page;
    uint32_t pagehdr_crc; /* random numbers? */
};

struct fs_file_hdr {
    uint16_t v_0x5001;
    uint8_t  empty; /* 0xFF if empty, 0xFC if not empty, 0xFE if the first empty block before the rest of the device is empty? */
#define HDR_EMPTY_ALLOCATED 0x1
#define HDR_EMPTY_MOREBLOCKS 0x2
    uint8_t  status;   /* bitfield; f4, f8, ... */
#define HDR_STATUS_VALID 0x1
#define HDR_STATUS_DEAD 0x2
#define HDR_STATUS_FILE_START 0x4
#define HDR_STATUS_FILE_CONT 0x8
    uint32_t rsvd_0; /* ff ff ff ff */
    uint32_t wear_level_counter;
    uint32_t rsvd_1;
    uint32_t rsvd_2;
    uint8_t  rsvd_3;
    uint8_t  next_page_crc;
    uint16_t next_page;
    uint32_t pagehdr_crc; /* random numbers? */
    
    uint32_t file_size;
    uint8_t  flag_2; /* FF or FE; FE if there's a filename */
#define HDR_FLAG_2_HAS_FILENAME 0x1
    uint8_t  filename_len;
    uint16_t rsvd_4; /* FF FF */
    uint32_t rsvd_5;
    uint32_t filehdr_crc;
    uint16_t st_tmp_file; /* non-zero if temp file, zero if not temp file */
    uint16_t st_create_complete; /* zero if create complete, non-zero if not */
    uint16_t st_delete_complete; /* zero if delete complete, non-zero if not */
    uint8_t  v_full[26];
};

/* assuming that no longer files are possible, just a guess */
#define MAX_FILENAME_LEN 32

struct fs_file_hdr_with_name {
    struct fs_file_hdr hdr;
    char name[MAX_FILENAME_LEN + 1];
};

uint32_t fs_pbfs_crc32(void *p, size_t len);
uint8_t fs_pbfs_crc8(void *p, size_t len);

uint32_t fs_pagehdr_crc(struct fs_page_hdr *hdr);
uint32_t fs_filehdr_crc(struct fs_file_hdr *hdr);
uint8_t  fs_next_page_crc(uint16_t next_page);
