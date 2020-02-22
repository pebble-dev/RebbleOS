# XXX: This is hokey as fuck.
from __main__ import Test

testplan = [
    Test("Simple", testname = b'simple', golden = 42),
    Test("bad-artifact", testname = b'simple', golden = 43),
    Test("non-exist", testname = b'ne', golden = 42),
    Test("RAMFS basic test", testname = b'ramfs_two_files'),
]
