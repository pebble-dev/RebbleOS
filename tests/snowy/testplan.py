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
    Test("bad-artifact", testname = b'simple', golden = 43),
    Test("non-exist", testname = b'ne', golden = 42),
    Test("RAMFS basic test", testname = b'ramfs_two_files'),
]
