#!/usr/bin/env python

"""
Builds a resource pack from a JSON description file.
RebbleOS

The general premise here is that there are two "and a half" output
products from mkpack:

  1) A pbpack file, which eventually goes inside of a pbz.  This gets
     flashed in one of two ways: either through the Pebble PRF flash tools
     (and, indirectly, from the Pebble apk or from libpebble2), or through
     a "fake flash" tool that constructs a $PLATFORM_spi.bin for QEMU.

  2) A C header file, which is included from RebbleOS.  Each resource
     compiled by mkpack is given an enum entry, and the C header file
     allows the resources to be references symbolically.

  2.5) A dependency file, to be used by a build system.  Ideally, make
       knows to ask mkpack to generate a dependency file; that way, a
       pbpack is rebuilt every time either the input JSON changes, or any
       of the resource pack inputs change.

mkpack is smart enough to know to get resources in a handful of ways.  It
can (aspirationally):

  * Extract a raw resource from another resource pack (handy for
    extracting fonts and such, before we have replacements of our own).
  * Import a raw resource from disk (including a PNG, or a font file).
  * Import an image from disk, converting and crushing to a palettized
    PNG.
  * Convert a graphic to system framebuffer format, for use as a splash
    screen.
"""

__author__ = "Joshua Wise <joshua@joshuawise.com>"

from stm32_crc import crc32
import struct
import json
import os
import sys

crush_png = None # To be imported

TAB_OFS = 0x0C
RES_OFS = 0x200C

def find_pebble_sdk():
    """
    Returns a valid path to the currently installed pebble sdk or 
    nothing if none was found.
    """

    for path in [
        "~/Library/Application Support/Pebble SDK/SDKs/current",
        "~/.pebble-sdk/SDKs/current"
    ]:
        if os.path.isdir(os.path.expanduser(path)):
            return os.path.expanduser(path)
    
    return None

def import_crush_png(sdk_path):
    """
    Activates the sdks virtual environment and dynamically imports
    the png2pblpng functionality
    """

    activate_this = os.path.join(sdk_path, ".env/bin/activate_this.py")
    exec(compile(open(activate_this, "rb").read(), activate_this, 'exec'), dict(__file__=activate_this))
    
    sys.path.append(os.path.join(sdk_path, "sdk-core/pebble/common/tools/"))
    from png2pblpng import convert_png_to_pebble_png_bytes
    return convert_png_to_pebble_png_bytes


def load_resource_from_pbpack(fname, resid):
    """
    Returns resource number |resid| from the pbpack file specified by
    |fname|.
    """
    
    with open(fname, 'rb') as f:
        nrsrc = struct.unpack('I', f.read(4))[0]
        
        for i in range(nrsrc):
            f.seek(TAB_OFS + i * 16)
            (idx, ofs, sz, crc) = struct.unpack('iiiI', f.read(16))
            
            if idx == resid:
                break
        else:
            raise IOError("res {} not found in pbpack \"{}\"".format(resid, fname))
        
        f.seek(RES_OFS + ofs)
        return f.read(sz)

def load_resource_from_disk(fname):
    """
    Simply loads a file.
    """
    
    with open(fname, 'rb') as f:
        return f.read()

def save_pbpack(fname, rsrcs):
    """
    Outputs a handful of resources to a file.
    
    |rsrcs| is a list of resources, with the first mapping to resource index
    "1".  Although the PebbleOS resource structure permits a sparse mapping
    -- i.e., one in which one must read the whole resource table to find the
    index that one desires -- the RebbleOS resource loader simply ignores
    the ID number in each table entry, and indexes directly in to find what
    it wants.  (And, further, every Pebble pbpack that I can find only has
    them in order.)  So we, in keeping, will only generate things like that.
    
    """
    
    # First, turn the resource table into a list of entries, including
    # index, offset, size, and CRC.
    def mk_ent(data):
        ent = {"idx": mk_ent.idx, "offset": mk_ent.offset, "size": len(data), "crc": crc32(data), "data": data}
        mk_ent.offset += len(data)
        mk_ent.idx += 1
        return ent
    mk_ent.offset = 0
    mk_ent.idx = 1
    rsrc_ents = [mk_ent(data) for data in rsrcs]
    
    with open(fname, 'wb+') as f:
        f.write(struct.pack('I', len(rsrc_ents))) # Number of resources.
        # We'll come back to the big CRC later.
        
        # Write out the table.
        f.seek(TAB_OFS)
        for ent in rsrc_ents:
            f.write(struct.pack('iiiI', ent["idx"], ent["offset"], ent["size"], ent["crc"]))
        
        # Write out the resources themselves.
        for ent in rsrc_ents:
            f.seek(RES_OFS + ent["offset"])
            f.write(ent["data"])
        
        # Now compute the CRC of the whole show.
        f.seek(RES_OFS)
        alldata = f.read()
        
        totlen = f.tell()
        
        f.seek(4)
        f.write(struct.pack('I', crc32(alldata)))
    
    return totlen

class Resource(object):
    def __init__(self, coll, j):
        self.coll = coll
        self.name = j["name"]

class ResourceRef(Resource):
    def __init__(self, coll, j):
        super(self.__class__, self).__init__(coll, j)
        
        self.resfile = self.coll.references[j["input"]["ref"]]
        self.resid = j["input"]["id"]
    
    def deps(self):
        return [self.resfile]
    
    def data(self):
        return load_resource_from_pbpack(self.resfile, self.resid)
    
    def sourcedesc(self):
        return "resource ID {} from pack {}".format(self.resid, self.resfile)

class ResourceFile(Resource):
    def __init__(self, coll, j):
        super(self.__class__, self).__init__(coll, j)
        
        self.file = "{}/{}".format(self.coll.root, j["input"]["file"])
    
    def deps(self):
        return [self.file]
    
    def data(self):
        return load_resource_from_disk(self.file)
    
    def sourcedesc(self):
        return self.file

class ResourceImage(Resource):
    def __init__(self, coll, j, palette):
        super(self.__class__, self).__init__(coll, j)
        
        self.file = "{}/{}".format(self.coll.root, j["input"]["file"])
        self.palette = palette
    
    def deps(self):
        return [self.file]
    
    def data(self):
        return crush_png(self.file, self.palette)
    
    def sourcedesc(self):
        return "imported image " + self.file

class ResourceCollection:
    """
    A collection of resources, as defined by a dictionary loaded from a JSON
    file.
    
    A resource JSON file is defined as being a JSON dictionary, with the
    following keys:
    
      * "references" is a dictionary, which maps friendly names to paths of
         resource packs.
         
      * "resources" is a list.  Each element of the list is a dictionary,
        describing one resource that will get included in the resource
        bundle.  This dictionary, in turn, contains the following keys:
        
          * "name": A symbol name, for the resource header file.
          
          * "input": A dictionary, describing where to include this resource
            from.  This dictionary should have a "type" key, which can be
            "file" or "resource": if "file", then there should be a "file"
            key, with a filename; if "resource", then there should be a
            "ref" key, with a reference from "references" above, and an "id"
            key, with a resource ID to load from that reference.
    
    """

    def __init__(self, fname, root = "."):
        """
        Load in a resource collection from a file, but don't load the
        resources associated with it.  (That happens later.)
        """
        
        self.jfname = fname
        self.root = root
        
        with open(fname, 'r') as f:
            jdb = json.load(f)
        
        if "references" in jdb:
            self.references = {k: "{}/{}".format(self.root, v) for k, v in list(jdb["references"].items())}
        else:
            self.references = {}

        if "palette" not in jdb:
            raise ValueError("palette is not set")
        
        self.resources = []
        for res in jdb["resources"]:
            if res["input"]["type"] == "file":
                self.resources.append(ResourceFile(self, res))
            elif res["input"]["type"] == "resource":
                self.resources.append(ResourceRef(self, res))
            elif res["input"]["type"] == "image":
                self.resources.append(ResourceImage(self, res, jdb["palette"]))
            else:
                raise ValueError("unknown resource type {}".format(res["type"]))
    
    def deps(self):
        """
        List of all dependencies of this resource pack.
        """
        
        deps = {}
        for r in self.resources:
            for d in r.deps():
                deps[d] = True
        return [d for d in deps]
    
    def rsrcs(self):
        """
        List of raw resource data in this resource pack.
        """
        
        return [r.data() for r in self.resources]
    
    def write_pbpack(self, fname):
        """
        Write out a .pbpack file with all of the resources loaded.
        """
        
        return save_pbpack(fname, self.rsrcs())
    
    def write_header(self, fname):
        """
        Write out a C header file that has symbolic names for resource
        identifiers.
        """
        
        with open(fname, 'w') as f:
            f.write("/* THIS FILE IS AUTOMATICALLY GENERATED BY mkpack.py. */\n")
            f.write("/* IF YOU MODIFY IT, YOU WILL BE VERY SAD. */\n")
            f.write("/* IF YOU CHECK IT IN, I WILL BE VERY SAD. */\n")
            f.write("\n")
            f.write("#pragma once\n")
            f.write("\n")
            f.write("typedef enum resource_id {\n")
            for (rid, r) in enumerate(self.resources):
                f.write("    {} = {}, /* (from {}) */\n".format(r.name, rid + 1, r.sourcedesc()))
            f.write("} resource_id;\n")
    
    def write_makedeps(self, fname, rsrcfile, hdrfile):
        """
        Write out a dependency file for 'make' to process.
        
        The generated header depends on the generated pbpack; the generated
        pbpack depends on the source files.
        """
        
        with open(fname, 'w') as f:
            f.write("# automatically generated by mkpack.py\n\n")
            f.write("{}: {} {} ".format(rsrcfile, self.jfname, __file__))
            for d in self.deps():
                f.write("{} ".format(d))
            f.write("\n\n")
            f.write("{}: {}\n\n".format(hdrfile, rsrcfile))
            for d in self.deps():
                f.write("{}:\n\n".format(d))
            f.write("# That will conclude this evening's entertainment.\n")


def main():
    """
    Command-line driver.
    """
    
    import argparse
    
    parser = argparse.ArgumentParser(description = "Resource pack generator for RebbleOS.")
    parser.add_argument("-r", "--root", nargs=1, default = ["."], help = "pathname to prepend to resource filenames.")
    parser.add_argument("-M", "--make-dep", action="store_true", default = False, help = "produce a .d file to be included by 'make'")
    parser.add_argument("-H", "--header", action = "store_true", default = False, help = "produce a .h file to be included in C source")
    parser.add_argument("-P", "--pbpack", action = "store_true", default = False, help = "produce a .pbpack file")
    parser.add_argument("-s", "--sdk", nargs=1, default = [None], help = "pathname to pebble sdk")
    parser.add_argument("json", help = "input JSON configuration file")
    parser.add_argument("basename", help = "base output name ('.d', '.h', and '.pbpack' are appended automatically)")
    args = parser.parse_args()

    sdk_path = find_pebble_sdk()
    if args.sdk[0] is not None:
        sdk_path = os.path.expanduser(args.sdk[0])
    if not os.path.isdir(sdk_path):
        raise ValueError("could not find pebble sdk, please provide one with --sdk")
    global crush_png
    crush_png = import_crush_png(sdk_path)
    
    rc = ResourceCollection(args.json, root = args.root[0])
    
    pbpack_name = "{}.pbpack".format(args.basename)
    header_name = "{}.h".format(args.basename)
    dep_name = "{}.d".format(args.basename)
    if args.pbpack:
        bytes = rc.write_pbpack(pbpack_name)
        print(("wrote {} ({} bytes)".format(pbpack_name, bytes)))
    if args.header:
        rc.write_header(header_name)
        print(("wrote {}".format(header_name)))
    if args.make_dep:
        rc.write_makedeps(dep_name, pbpack_name, header_name)
        print(("wrote {}".format(dep_name)))

if __name__ == '__main__':
    main()
