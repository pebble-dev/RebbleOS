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

# Load the testplan.
sys.path.append(f'tests/{args.platform[0]}')
from testplan import testplan

import shlex
import subprocess, signal
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

class Emulator:
    def __init__(self, image = None):
        if image == None:
            image = make_image()
        
        nargs = shlex.split(args.qemu[0])
        nargs += [image.name]
        
        self.qemu = subprocess.Popen(nargs, stderr = subprocess.STDOUT, stdout = subprocess.PIPE)
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
        
        self.transport = QemuTransport(socket = skt)

        target,data = self.recv()
        assert(isinstance(data, QemuRebbleTest))
        assert(isinstance(data.payload, QemuRebbleTestAlive))
    
    def recv(self):
        return self.transport.read_packet()
    
    def send(self, payload):
        self.transport.send_packet(QemuRebbleTest(payload = payload), target = MessageTargetQemu(protocol = 100))
        
    def kill(self):
        if self.qemu is not None:
            self.qemu.send_signal(signal.SIGINT)
            self.logs = self.qemu.communicate()[0]
        self.qemu = None

    def __enter__(self):
        return self

    def __exit__(self, exc, value, tb):
        self.kill()
        if exc:
            print("*** Emulator stopped because of exception ***")
            print("Emulator logs:")
            print(self.logs.decode('iso-8859-1', 'ignore')) # Ugh.  This will bite me some day.
        return False
    
    def __del__(self):
        self.kill()

def load_tests():
    tests = {}
    
    with make_image(f'tests/{args.platform[0]}/blank.gz') as f, Emulator(f) as e:
        print("Asking for test list...")
        e.send(QemuRebbleTestListRequest())
        while True:
            target,data = e.recv()
            assert(isinstance(data, QemuRebbleTest))
            assert(isinstance(data.payload, QemuRebbleTestListResponse))
            print(data.payload)
            print(f"Test {data.payload.id}: {str(data.payload.name)}")
            tests[data.payload.name] = data.payload.id
            if data.payload.is_last_test == 1:
                break
    
    return tests

tests = load_tests()

print("Running test...")
with make_image(f'tests/{args.platform[0]}/blank.gz') as f, Emulator(f) as e:
    e.send(QemuRebbleTestRunRequest(id = tests[b'simple']))
    target,data = e.recv()
    print(data.payload)

