/* fs_test.c
 * Tests for RebbleOS filesystem
 * RebbleOS
 */

#include "fs.h"
#include "test.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

#ifdef REBBLEOS_TESTING

#define LFSR_INIT 0xFFFFFFFF
#define LFSR_POLY 0x04C11DB7
static uint8_t lfsr_next(uint32_t *lfsr) {
    if (*lfsr & 0x80000000)
        *lfsr = (*lfsr << 1) ^ LFSR_POLY;
    else
        *lfsr =  *lfsr << 1;
    return *lfsr & 0xFF;
}

static int bigfile_make(const char *name, size_t sz, uint32_t seed) {
#define IOBUFSIZ 64
    uint8_t buf[IOBUFSIZ];
    uint32_t lfsr = seed;
    
    struct fd fd, *fdp;
    fdp = fs_creat(&fd, name, sz);
    if (!fdp)
        return 1;
    fs_mark_written(fdp);
    
    int ofs = 0;
    while (sz) {
        int csz = (sz < IOBUFSIZ) ? sz : IOBUFSIZ;
        int i;
        
        for (i = 0; i < csz; i++) {
            buf[i] = lfsr_next(&lfsr);
        }
        
        if (ofs == 0)
            printf("  starts with: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
        
        fs_write(&fd, buf, csz);
        
        sz -= csz;
        ofs += csz;
    }
    
    return 0;
}

static int bigfile_verify(const char *name, size_t sz, uint32_t seed) {
#define IOBUFSIZ 64
    uint8_t buf[IOBUFSIZ];
    uint32_t lfsr = seed;

    struct file file;
    struct fd fd, *fdp;
    int rv;
    
    rv = fs_find_file(&file, name);
    if (rv < 0)
        return 1;
    fs_open(&fd, &file);
    
    int ofs = 0;
    
    while (sz) {
        int csz = (sz < IOBUFSIZ) ? sz : IOBUFSIZ;
        int i;
        
        rv = fs_read(&fd, buf, csz);
        if (rv < csz) {
            printf("short read on file %s at ofs %d: exp %d, got %d\n", name, ofs, csz, rv);
            return 2;
        }
        
        if (ofs == 0)
            printf("  starts with: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
        
        for (i = 0; i < csz; i++) {
            uint8_t ex = lfsr_next(&lfsr);
            if (buf[i] != ex) {
                printf("read error on file %s at ofs %d: exp %d, got %d\n", name, ofs + i, ex, buf[i]);
                return 3;
            }
        }
        
        ofs += csz;
        sz -= csz;
    }
    
    return 0;
}

TEST(fs_bigfiles) {
    int rv;
    
    printf("making bigfile1 ...\n");
    rv = bigfile_make("bigfile/1", 31415, 92653);
    if (rv != 0) {
        *artifact = 1000 + rv;
        return TEST_FAIL;
    }
    
    printf("making bigfile2 ...\n");
    rv = bigfile_make("bigfile/2", 27182, 81828);
    if (rv != 0) {
        *artifact = 2000 + rv;
        return TEST_FAIL;
    }
    
    printf("verifying bigfile1 ...\n");
    rv = bigfile_verify("bigfile/1", 31415, 92653);
    if (rv != 0) {
        *artifact = 3000 + rv;
        return TEST_FAIL;
    }

    printf("verifying bigfile2 ...\n");
    rv = bigfile_verify("bigfile/2", 27182, 81828);
    if (rv != 0) {
        *artifact = 4000 + rv;
        return TEST_FAIL;
    }
    
    *artifact = 0;
    return TEST_PASS;
}

TEST(fs_creat_basic) {
    struct fd fd, *fdp;
    struct file file;
    int rv;
    
    fdp = fs_creat(&fd, "testfile", 16384);
    if (!fdp) {
        *artifact = 1;
        return TEST_FAIL;
    }
    
    fs_mark_written(fdp);
    
    rv = fs_find_file(&file, "testfile");
    if (rv < 0) {
        *artifact = 2;
        return TEST_FAIL;
    }
    
    *artifact = 0;
    return TEST_PASS;
}

TEST(fs_find_noent) {
    struct file file;
    int rv = fs_find_file(&file, "noent");
    *artifact = rv;
    return (rv < 0) ? TEST_PASS : TEST_FAIL;
}

TEST(fs_two_files) {
    struct fd fd1, fd2, *fdp;
    struct file file1, file2;
    int rv = TEST_PASS;
    
    *artifact = 0;
    
    fdp = fs_creat(&fd1, "hello", 5);
    if (!fdp) { *artifact = 1; return TEST_FAIL; }
    fs_write(&fd1, "Hello", 5);
    fs_mark_written(fdp);
    
    fdp = fs_creat(&fd2, "world", 5);
    if (!fdp) { *artifact = 2; return TEST_FAIL; }
    fs_write(&fd2, "World", 5);
    fs_mark_written(fdp);
    
    char buf[5];

    rv = fs_find_file(&file1, "world");
    if (rv < 0) { *artifact = 3; return TEST_FAIL; }
    fs_open(&fd1, &file1);
    rv = fs_read(&fd1, buf, 5);
    if (rv < 5) { *artifact = 4; return TEST_FAIL; }
    if (memcmp(buf, "World", 5)) { *artifact = 5; return TEST_FAIL; }

    rv = fs_find_file(&file1, "hello");
    if (rv < 0) { *artifact = 6; return TEST_FAIL; }
    fs_open(&fd1, &file1);
    rv = fs_read(&fd1, buf, 5);
    if (rv < 5) { *artifact = 7; return TEST_FAIL; }
    if (memcmp(buf, "Hello", 5)) { *artifact = 8; return TEST_FAIL; }
    
    *artifact = 0;
    return TEST_PASS;
}
/*
Tests the file_replacing logic,
in all of its forms (create a file with contents A; create a file replacing that file with contents B;
open the file for read and make sure you get contents A; do the 'finish replacing' operation;
open the file for read and make sure you get contents B;
*/
TEST(fs_replace_file_basic) {
    struct fd fd, *fdp;
    struct file file;
    int rv = TEST_PASS;

    *artifact = 0;

    //create a file with contents a
    fdp = fs_creat(&fd, "hello", 5);
    if (!fdp) { *artifact = 1; return TEST_FAIL; }
    fs_write(&fd, "Hello", 5);
    fs_mark_written(fdp);

    //create a file replacing that file with contents B
    rv = fs_find_file(&file, "hello");
    if (rv < 0) { *artifact = 2; return TEST_FAIL; }
    fdp = fs_creat_replacing(&fd, "hello", 5, &file);
    fs_write(&fd, "World", 5);

    //open the file for read and make sure you get contents A;
    char buf[5];

    rv = fs_find_file(&file, "hello");
    if (rv < 0) { *artifact = 3; return TEST_FAIL; }

    fs_open(&fd, &file);
    rv = fs_read(&fd, buf, 5);
    if (memcmp(buf, "Hello", 5)) { *artifact = 4; return TEST_FAIL; }

    //do the 'finish replacing' operation;
    fs_mark_written(fdp);

    //open the file for read and make sure you get contents B;
    rv = fs_find_file(&file, "hello");
    if (rv < 0) { *artifact = 5; return TEST_FAIL; }

    fs_open(&fd, &file);
    rv = fs_read(&fd, buf, 5);
    if (memcmp(buf, "World", 5)) { *artifact = 6; return TEST_FAIL; }

    *artifact = 0;
    return TEST_PASS;
}


TEST(fs_sub_file) {
    struct fd fd, *fdp;
    struct file file, file2;
    int rv;
    
    fdp = fs_creat(&fd, "hello", 5);
    if (!fdp) { *artifact = 1; return TEST_FAIL; }
    fs_write(fdp, "Hello", 5);
    fs_mark_written(fdp);
    
    uint8_t buf[10];
    
    rv = fs_find_file(&file, "hello");
    if (rv < 0) { *artifact = 2; return TEST_FAIL; }
    fs_open(&fd, &file);
    rv = fs_read(&fd, buf, 5);
    if (rv != 5) { *artifact = 3; return TEST_FAIL; }
    if (memcmp(buf, "Hello", 5)) { *artifact = 4; return TEST_FAIL; }
    
    fs_file_from_file(&file2, &file, 1, 3);
    fs_open(&fd, &file2);
    rv = fs_read(&fd, buf, 10);
    if (rv != 3) { *artifact = 5; return TEST_FAIL; }
    if (memcmp(buf, "ell", 3)) { *artifact = 6; return TEST_FAIL; }
    
    *artifact = 0;
    return TEST_PASS;
}


#endif
