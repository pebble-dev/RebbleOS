#!/usr/bin/env python3

"""
Launches one or more QEMUs and runs tests as needed.
RebbleOS
"""

import argparse
import socket
import sys
import struct
from libpebble2.communication.transports.qemu.protocol import *
from libpebble2.communication.transports.qemu import QemuTransport, MessageTargetQemu

parser = argparse.ArgumentParser(description = "Test runner for RebbleOS.")
parser.add_argument("--qemu", nargs = 1, required = True, help = "QEMU command, not including SPI flash image")
parser.add_argument("--platform", nargs = 1, required = True, help = "platform name for SPI images")
args = parser.parse_args()

import shlex
import subprocess
import time

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
    
    return qemu, skt
  
q, skt = launch_qemu(f'build/{args.platform[0]}_test/fw.qemu_spi.bin')

t = QemuTransport(socket = skt)

def send_data(d):
    t.send_packet(QemuRebbleTest(payload = d), target = MessageTargetQemu(protocol = 100))

def recv_data():
    return t.read_packet()

print("Waiting for QEMU to wake up...")
target,data = t.read_packet()
assert(isinstance(data, QemuRebbleTest))
assert(isinstance(data.payload, QemuRebbleTestAlive))

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
