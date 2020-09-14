#!/usr/bin/env python3

"""
Time zone compiler, from TZif format to RebbleOS resources.

This tool produces a RebbleOS time zone file from the IANA time zone table. 
RebbleOS time zones are limited inasmuch as they only support Gregorian /
Julian daylight savings time transitions, and only support the current time
zone -- that is to say, the input timezone file must have exactly one
transition.  Since there is only one transition, that transition is ignored
and the offset from the long-form timezone string is used.

The RebbleOS time zone file format is a packed binary database with a list
of records.  A record can be:

A byte, 0x01, followed by a 1-byte length (including 0-terminator), followed
by a 0-terminated string -- this is a folder name.

A byte, 0x02, followed by a 1-byte length (include 0-terminator), followed
by a 0-terminated string -- this is a time zone name.  Following a time zone
name is a time zone record:

  union tzrec {
    unsigned char hasdst;
    struct {
      unsigned char hasdst; /* 0 */
      long offset; /* in seconds from GMT */
    } tz_without_dst;
    struct {
      unsigned char hasdst; /* 1 */
      long offset; /* in seconds from GMT */
      long dstoffset; /* in seconds from GMT */
      unsigned char dst_start_mode;
      unsigned short dst_start_param[3];
      unsigned long dst_start_time; /* in seconds from the start of the day */
      unsigned char dst_end_mode;
      unsigned short dst_end_param[3];
      unsigned long dst_end_time; /* in seconds from the start of the day */
    } tz_with_dst;
  };

dst_start_mode (and dst_end_mode) can be:

  0: Julian day (1 to 365, inclusive).  February 29th cannot be referred to. 
  1: Zero-based Julian day (0 to 365, inclusive).  Leap days are counted. 
  2: Month, week, day.  Months are 1-based (1 to 12).  Weeks are 1-based;
     week 5 is always "the last week of the month".  Days are zero-based
     (ha, gotcha!) from Sunday.

"""

import os
import sys
import struct
import argparse

class Tzif:
    HDRFMT = '!4sc15xIIIIII'
    MODE_JULIAN  = 0
    MODE_JULIANZ = 1
    MODE_MND     = 2
    def __init__(self, filename):
        with open(filename, 'rb') as fd:
            hdr = fd.read(struct.calcsize(self.HDRFMT))
            magic, vers, isutcnt, isstdcnt, leapcnt, timecnt, typecnt, charcnt = \
                struct.unpack(self.HDRFMT, hdr)
            if magic != b'TZif':
                raise ValueError(f"unsupported tzif magic {magic}")
            if vers != b'2' and vers != b'3':
                raise ValueError(f"unsupported tzif vers {vers}")
            
            # We know it's a tzif2 file, so skip the tzif1 header.
            fd.seek(
                timecnt * 4 +
                timecnt +
                typecnt * 6 + 
                charcnt +
                leapcnt * 8 +
                isstdcnt +
                isutcnt
                , 1)
            
            hdr = fd.read(struct.calcsize(self.HDRFMT))
            magic, vers, isutcnt, isstdcnt, leapcnt, timecnt, typecnt, charcnt = \
                struct.unpack(self.HDRFMT, hdr)
            if magic != b'TZif':
                raise ValueError(f"unsupported tzif magic {magic}")
            if vers != b'2' and vers != b'3':
                raise ValueError(f"unsupported tzif vers {vers}")
            
            self.trntimes = [struct.unpack("!q", fd.read(8))[0]   for _ in range(timecnt)]
            self.trntypes = [struct.unpack("!B", fd.read(1))[0]   for _ in range(timecnt)]
            self.trnrecs  = [struct.unpack("!lBB", fd.read(6))    for _ in range(typecnt)]
            self.sbuf     = fd.read(charcnt)
            self.leaps    = [struct.unpack("!ql", fd.read(12))    for _ in range(leapcnt)]
            if isstdcnt != 0:
                raise ValueError(f"unsupported isstdcnt {isstdcnt}")
            if isutcnt != 0:
                raise ValueError(f"unsupported isutcnt {isutcnt}")
            
            
            foot0 = fd.readline()
            if foot0 != b'\n':
                raise ValueError(f"unsupported footer 0 {foot0}")
            
            tzstr = fd.readline().strip()
            def eatdesig():
                nonlocal tzstr
                desig = b""
                if tzstr[0:1] == b'<':
                    while tzstr[0:1] != b'>':
                        desig += tzstr[0:1]
                        tzstr = tzstr[1:]
                    desig += tzstr[0:1]
                    tzstr = tzstr[1:]
                else:
                    while tzstr[0:1].isalpha():
                        desig += tzstr[0:1]
                        tzstr = tzstr[1:]
                return desig
            
            def eatofs():
                nonlocal tzstr
                sign = 1
                grp = 0
                hms = [0, 0, 0]
                if tzstr[0:1] == b'+':
                    tzstr = tzstr[1:]
                elif tzstr[0:1] == b'-':
                    sign = -1
                    tzstr = tzstr[1:]
                
                while tzstr[0:1] == b':' or tzstr[0:1].isdigit():
                    if tzstr[0:1].isdigit():
                        hms[grp] *= 10
                        hms[grp] += tzstr[0] - b'0'[0]
                    else:
                        grp += 1
                    tzstr = tzstr[1:]
                return sign * hms[0] * 60 * 60 + hms[1] * 60 + hms[2]
            
            def eatdate():
                nonlocal tzstr
                if tzstr[0:1] == b'M':
                    mnd = [0, 0, 0]
                    grp = 0
                    tzstr = tzstr[1:]
                    while tzstr[0:1] == b'.' or tzstr[0:1].isdigit():
                        if tzstr[0:1].isdigit():
                            mnd[grp] *= 10
                            mnd[grp] += tzstr[0] - b'0'[0]
                        else:
                            grp += 1
                        tzstr = tzstr[1:]
                    return (self.MODE_MND, mnd[0], mnd[1], mnd[2], )
                
                mode = self.MODE_JULIANZ
                if tzstr[0:1] == b'J':
                    tzstr = tzstr[1:]
                    mode = self.MODE_JULIAN
                
                jval = 0
                while tzstr[0:1].isdigit():
                    jval *= 10
                    jval += tzstr[0] - b'0'[0]
                    tzstr = tzstr[1:]
                
                return (mode, jval, 0, 0, )
            
            self.std = eatdesig()
            self.ofs = eatofs()
            self.dst = None
            self.dstofs = None
            if len(tzstr) > 0 and tzstr[0:1] != b',':
                self.dst = eatdesig()
                self.dstofs = self.ofs - 3600
            if len(tzstr) > 0 and tzstr[0:1] != b',':
                self.dstofs = eatofs()
            
            if tzstr[0:1] == b',':
                tzstr = tzstr[1:]
                self.datestart = eatdate()
                self.timestart = 2 * 60 * 60
                if tzstr[0:1] == b'/':
                    tzstr = tzstr[1:]
                    self.timestart = eatofs()
                if tzstr[0:1] != b',':
                    raise ValueError(f"only half of a dst rule? {tzstr}")
                tzstr = tzstr[1:]
                self.dateend = eatdate()
                self.timeend = 2 * 60 * 60
                if tzstr[0:1] == b'/':
                    tzstr = tzstr[1:]
                    self.timeend = eatofs()

    def print(self):
        print(f"{self.std}, offset {self.ofs}")
        if self.dst is not None:
            print(f"  Daylight: {self.dst}, offset {self.dstofs}, starts {self.datestart} at {self.timestart}, ends {self.dateend} at {self.timeend}")
        for trn in range(len(self.trntimes)):
            rec = self.trnrecs[self.trntypes[trn]]
            name = b""
            npos = rec[2]
            while self.sbuf[npos] != 0:
                name += bytes([self.sbuf[npos]])
                npos += 1
            print(f"trn {trn} at time {self.trntimes[trn]}, utoff {rec[0]}, isdst {rec[1]}, name {name}")
        for l in self.leaps:
            print(f"leap at time {l[0]}, del {l[1]}")

parser = argparse.ArgumentParser(description = "Timezone file builder for RebbleOS.")
parser.add_argument("-i", "--input", nargs = 1, required = True, help = "input path")
parser.add_argument("-o", "--output", nargs = 1, required = True, help = "compiled resource output file")
args = parser.parse_args()

tzpaths = [
    'Africa',
    'America',
    'Antarctica',
    'Asia',
    'Atlantic',
    'Australia',
    'Europe',
    'Indian',
    'Pacific'
]

out = open(args.output[0], 'wb')
for dir in tzpaths:
    dirpath = os.path.join(args.input[0], dir)
    out.write(b'\x01' + struct.pack('B', len(dir.encode()) + 1) + dir.encode() + b'\x00')
    fs = [f for f in os.listdir(dirpath)]
    fs.sort()
    for f in fs:
        try:
            tzf = os.path.join(dirpath, str(f))
            if not os.path.isfile(tzf):
                continue
            tz = Tzif(tzf)
            if len(tz.trntimes) != 1:
                raise ValueError(f"unsupported timecnt {len(tz.trntimes)}")
            
            out.write(b'\x02' + struct.pack('B', len(f.encode()) + 1) + f.encode() + b'\x00')
            
            if tz.dst is None:
                out.write(struct.pack("<Bl", 0, tz.ofs))
            else:
                out.write(struct.pack("<BllBhhhlBhhhl",
                    1, tz.ofs, tz.dstofs,
                    tz.datestart[0], tz.datestart[1], tz.datestart[2], tz.datestart[3], tz.timestart,
                    tz.dateend  [0], tz.dateend  [1], tz.dateend  [2], tz.dateend  [3], tz.timeend))
        except Exception as e:
            print(f"Failed to load {dir}/{f}, {e}")
