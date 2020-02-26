/* fs_test.c
 * Tests for RebbleOS filesystem
 * RebbleOS
 */

#include "fs.h"
#include "test.h"
#include "debug.h"

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
