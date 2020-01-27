#!/usr/bin/env python3

"""
Launches one or more QEMUs and runs tests as needed.
RebbleOS
"""

import argparse
import socket
import sys
import struct
import tempfile
from libpebble2.communication.transports.qemu.protocol import *
from libpebble2.communication.transports.qemu import QemuTransport, MessageTargetQemu

parser = argparse.ArgumentParser(description = "Test runner for RebbleOS.")
parser.add_argument("--qemu", nargs = 1, required = True, help = "QEMU command, not including SPI flash image")
parser.add_argument("--platform", nargs = 1, required = True, help = "platform name for SPI images")
parser.add_argument("--fsofs", nargs = 1, required = True, help = "offset into SPI image of filesystem")
parser.add_argument("--fssize", nargs = 1, required = True, help = "size of filesystem inside of SPI image")
parser.add_argument("--flashsize", nargs = 1, required = True, help = "total SPI flash size")
parser.add_argument("--resofs", nargs = 1, required = True, help = "offset into SPI image of resource pack")

args = parser.parse_args()

import shlex
import subprocess
import time
import gzip

def make_image(fs = None, keep = False):
    newimg = tempfile.NamedTemporaryFile(delete = not keep)
    newimg.write(b'\xff' * int(args.flashsize[0]))
    
    # Add in a resource pack.
    with open(f'build/{args.platform[0]}_test/res/{args.platform[0]}_res.pbpack', 'rb') as resimg:
      resdata = resimg.read()
    newimg.seek(int(args.resofs[0]))
    newimg.write(resdata)
    
    # Optionally, add a filesystem.
    if fs is not None:
        with gzip.open(fs, 'rb') as fsimg:
            fsdata = fsimg.read()
        newimg.seek(int(args.fsofs[0]))
        newimg.write(fsdata)
    
    return newimg

def launch_qemu(image):
    nargs = shlex.split(args.qemu[0])
    nargs += [image]
    qemu = subprocess.Popen(nargs)
  
    # Wait for the emulator to launch.
    launched = False
    for _ in range(50):
        try:
            skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            skt.connect(('127.0.0.1', 63771))
            launched = True
            break
        except socket.error as e:
            time.sleep(0.1)
    if not launched:
        raise Exception("QEMU did not launch in five seconds")

    print("Waiting for QEMU to wake up...")
    t = QemuTransport(socket = skt)

    target,data = t.read_packet()
    assert(isinstance(data, QemuRebbleTest))
    assert(isinstance(data.payload, QemuRebbleTestAlive))
    
    return qemu, t

with make_image(f'tests/{args.platform[0]}/blank.gz') as f:
    q, t = launch_qemu(f.name)

def send_data(d):
    t.send_packet(QemuRebbleTest(payload = d), target = MessageTargetQemu(protocol = 100))

def recv_data():
    return t.read_packet()

print("Asking for test list...")
send_data(QemuRebbleTestListRequest())
while True:
    target,data = t.read_packet()
    assert(isinstance(data, QemuRebbleTest))
    assert(isinstance(data.payload, QemuRebbleTestListResponse))
    print(data.payload)
    print(f"Test {data.payload.id}: {str(data.payload.name)}")
    if data.payload.is_last_test == 1:
        break

print("Running test...")
send_data(QemuRebbleTestRunRequest(id = 1))
target,data = t.read_packet()
print(data.payload)

q.terminate()
