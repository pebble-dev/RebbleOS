from rebbletest import Platform, Test

class SnowyPlatform(Platform):
    def __init__(self, qemu):
        super().__init__(
            name = 'snowy',
            flashsize = 16777216,
            resofs = 3670016,
            fsofs = 4194304,
            fssize = 12582912,
            qemu = qemu
        )

ThisPlatform = SnowyPlatform

testplan = [
    Test("Simple", testname = b'simple', golden = 42),
#    Test("bad-artifact", testname = b'simple', golden = 43),
#    Test("non-exist", testname = b'ne', golden = 42),
    Test("RAMFS basic test", testname = b'ramfs_two_files'),
    Test("Filesystem: find nonexistent file", testname = b'fs_find_noent', golden = 0),
    Test("Filesystem: basic create test", testname = b'fs_creat_basic', golden = 0),
    Test("Filesystem: I/O on two files", testname = b'fs_two_files', golden = 0),
    Test("Filesystem: basic file replacement test", testname = b'fs_replace_file_basic', golden = 0),
    Test("Blobdb: basic", testname = b'blobdb_basic', golden = 0),
]
