#!/usr/bin/env python

"""
Builds a firmware image zipball.
RebbleOS
"""

__author__ = "Joshua Wise <joshua@joshuawise.com>"

from stm32_crc import crc32
import json
import argparse
import zipfile
import time
import os

parser = argparse.ArgumentParser(description = "Firmware image builder for RebbleOS.")
parser.add_argument("-p", "--platform", nargs = 1, default = ["snowy_dvt"], help = "hardware codename")
parser.add_argument("-c", "--commit", nargs = 1, default = None, help = "commit SHA hash")
parser.add_argument("-v", "--version", nargs = 1, default = None, help = "firmware version")
parser.add_argument("-l", "--license", nargs = 1, default = None, help = "license file")
parser.add_argument("firmware", help = "tintin_fw.bin image")
parser.add_argument("respack", help = "resource pack")
parser.add_argument("pbz", help = "output PBZ file")
args = parser.parse_args()

FIRMWARE_NAME = "tintin_fw.bin"
RESPACK_NAME = "system_resources.pbpack"

manifest = {
  "manifestVersion": 2,
  "firmware": {
    "name": FIRMWARE_NAME,
    "hwrev": args.platform[0], 
    "type": "normal",
  },
  "generatedBy": "love and rockets",
  "generatedAt": int(time.time()),
  "debug": {},
  "type": "firmware",
  "resources": {
    "name": RESPACK_NAME
  }
}

if args.commit:
  manifest["firmware"]["commit"] = args.commit[0]
if args.version:
  manifest["firmware"]["versionTag"] = args.version[0]

def populate(manifest, what, filename):
  with open(filename, "rb") as f:
    stat = os.fstat(f.fileno())
    data = f.read()
  manifest[what]["crc"] = crc32(data)
  manifest[what]["size"] = len(data)
  manifest[what]["timestamp"] = int(stat.st_mtime)
  return data

fw_data = populate(manifest, "firmware", args.firmware)
res_data = populate(manifest, "resources", args.respack)

z = zipfile.ZipFile(args.pbz, "w", zipfile.ZIP_STORED)
if args.license:
  z.write(args.license[0], "LICENSE")
z.writestr(FIRMWARE_NAME, fw_data)
z.writestr(RESPACK_NAME, res_data)
z.writestr("manifest.json", json.dumps(manifest, indent=4, separators=(',', ': ')))
z.close()
