#!/usr/bin/env python3

"""
Launches one or more QEMUs and runs tests as needed.
RebbleOS
"""

import argparse
import socket
import sys
import struct

parser = argparse.ArgumentParser(description = "Test runner for RebbleOS.")
parser.add_argument("--qemu", nargs = 1, required = True, help = "QEMU command, not including SPI flash image")
parser.add_argument("--platform", nargs = 1, required = True, help = "platform name for SPI images")
parser.add_argument("--fsofs", nargs = 1, required = True, help = "offset into SPI image of filesystem")
parser.add_argument("--fssize", nargs = 1, required = True, help = "size of filesystem inside of SPI image")
parser.add_argument("--flashsize", nargs = 1, required = True, help = "total SPI flash size")
parser.add_argument("--resofs", nargs = 1, required = True, help = "offset into SPI image of resource pack")

args = parser.parse_args()

# Load the testplan.
sys.path.append(f'tests/{args.platform[0]}')
from testplan import testplan

from rebbletest import *

plat = Platform(name = args.platform[0], flashsize = int(args.flashsize[0]), resofs = int(args.resofs[0]), fsofs = int(args.fsofs[0]), qemu = args.qemu[0])

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

