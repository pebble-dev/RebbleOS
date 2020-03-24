__author__ = 'jwise'

import shlex
import subprocess, signal
import time
import gzip
import tempfile
import socket

from libpebble2.communication.transports.qemu.protocol import *
from libpebble2.communication.transports.qemu import QemuTransport, MessageTargetQemu

class Emulator:
    def __init__(self, qemu, image, debug = False):
        self.debug = debug
        
        nargs = shlex.split(qemu)
        nargs += [image.name]
        if not debug:
            nargs += ["-display", "none"]
        else:
            nargs += ["-S"]
        
        if not debug:
            self.qemu = subprocess.Popen(nargs, stderr = subprocess.STDOUT, stdout = subprocess.PIPE)
        else:
            self.qemu = subprocess.Popen(nargs)
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
            if self.debug:
                self.logs = b""
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

class Platform:
    def __init__(self, name, flashsize, resofs, fsofs, fssize, qemu):
        self.name = name
        self.flashsize = flashsize
        self.resofs = resofs
        self.fsofs = fsofs
        self.fssize = fssize
        self.qemu = qemu
        self.debug = False
        self.testmap = {}

    @property
    def respack(self):
        return f'build/{self.name}_test/res/{self.name}_res.pbpack'
    
    def image_path(self, imgname):
        return f'tests/{self.name}/{imgname}.gz'
    
    def make_image(self, fs = None, keep = False):
        newimg = tempfile.NamedTemporaryFile(delete = not keep)
        newimg.write(b'\xff' * self.flashsize)
    
        # Add in a resource pack.
        with open(self.respack, 'rb') as resimg:
          resdata = resimg.read()
        newimg.seek(self.resofs)
        newimg.write(resdata)
        
        # Optionally, add a filesystem.
        if fs is not None:
            with gzip.open(fs, 'rb') as fsimg:
                fsdata = fsimg.read()
            newimg.seek(self.fsofs)
            newimg.write(fsdata)
    
        return newimg
    
    def launch(self, image = None):
        with self.make_image(image) as im:
            return Emulator(image = im, qemu = self.qemu, debug = self.debug)
    
    def load_tests(self):
        self.testmap = {}
    
        with self.launch() as e:
            e.send(QemuRebbleTestListRequest())
            while True:
                target,data = e.recv()
                assert(isinstance(data, QemuRebbleTest))
                assert(isinstance(data.payload, QemuRebbleTestListResponse))
                self.testmap[data.payload.name] = data.payload.id
                if data.payload.is_last_test == 1:
                    break

class TestConfigurationException(Exception):
    pass

class TestFailureException(Exception):
    pass

class Test:
    def __init__(self, name, testname = None, image = 'blank', golden = None):
        self.name = name
        self.testname = testname
        self.image = image
        self.golden = golden
    
    def run(self, platform):
        if self.testname not in platform.testmap:
            raise TestConfigurationException(f"{self.testname} does not exist in running system!")
        
        data = None
        with platform.launch(platform.image_path(self.image)) as e:
            e.send(QemuRebbleTestRunRequest(id = platform.testmap[self.testname]))
            target,data = e.recv()

            if data.payload.passed == 0:
                raise TestFailureException(f"Test reported failure with artifact {data.payload.artifact}")
            if self.golden and data.payload.artifact != self.golden:
                raise TestFailureException(f"Test reported pass, but artifact {data.payload.artifact} differs from golden {self.golden}")
        
        return data.payload.artifact

