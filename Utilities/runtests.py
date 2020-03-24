#!/usr/bin/env python3

"""
Test driver to launches one or more QEMUs and run tests as needed.
RebbleOS
"""

import argparse
import socket
import sys
import struct

from rebbletest import TestFailureException, TestConfigurationException

parser = argparse.ArgumentParser(description = "Test runner for RebbleOS.")
parser.add_argument("--qemu", nargs = 1, required = True, help = "QEMU command, not including SPI flash image")
parser.add_argument("--platform", nargs = 1, required = True, help = "platform name for testplan")
parser.add_argument("--only", nargs = 1, help = "Run only one test.")
parser.add_argument("--debug", action = "store_true", default = False, help = "Run tests in debug mode.")

args = parser.parse_args()

# Load the testplan.
sys.path.append(f'tests/{args.platform[0]}')
from testplan import ThisPlatform, testplan

plat = ThisPlatform(qemu = args.qemu[0])

print("Reading tests...")
plat.load_tests()
print(f"... loaded {len(plat.testmap)} tests.")

if args.debug:
    plat.debug = True

passed,failed = 0,0
for t in testplan:
    if args.only and t.name not in args.only:
        print(f"Skipped \"{t.name}\".")
        continue
    print(f"Running \"{t.name}\"...")
    try:
        t.run(plat)
        print("PASSED")
        passed += 1
    except (TestFailureException, TestConfigurationException) as e:
        print(f"FAILED: {e}")
        failed += 1

print(f"*** {passed} test(s) passed, {failed} test(s) failed. ***")

