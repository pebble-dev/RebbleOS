#!/usr/bin/env python3

"""
Test driver to launches one or more QEMUs and run tests as needed.
RebbleOS
"""

import argparse
import socket
import sys
import struct

parser = argparse.ArgumentParser(description = "Test runner for RebbleOS.")
parser.add_argument("--qemu", nargs = 1, required = True, help = "QEMU command, not including SPI flash image")
parser.add_argument("--platform", nargs = 1, required = True, help = "platform name for testplan")

args = parser.parse_args()

# Load the testplan.
sys.path.append(f'tests/{args.platform[0]}')
from testplan import ThisPlatform, testplan

plat = ThisPlatform(qemu = args.qemu[0])

print("Reading tests...")
plat.load_tests()
print(f"... loaded {len(plat.testmap)} tests.")

passed,failed = 0,0
for t in testplan:
    print(f"Running \"{t.name}\"...")
    res = t.run(plat)
    if res is None:
        print("PASSED")
        passed += 1
    else:
        print(f"FAILED: {res}")
        failed += 1

print(f"*** {passed} test(s) passed, {failed} test(s) failed. ***")

