#!/usr/bin/env python
import QBUpgradeCatalog
from optparse import OptionParser
import traceback
import sys
import os


def make_sections(catalog_file_name, output_file_name):
    # try to load existing catalog file
    catalog = QBUpgradeCatalog.Catalog()
    if catalog_file_name == "-":
        print >> sys.stderr, "> loading catalog from standard input"
        catalog.read(sys.stdin)
    else:
        print >> sys.stderr, "> loading catalog from file '%s'" % catalog_file_name
        with open(catalog_file_name, "r") as f:
            catalog.read(f)
    print >> sys.stderr, "> done, found %d object(s)" % len(catalog.objects)
    print >> sys.stderr, "> verifying:"
    cnt = len(catalog.objects)
    catalog.verify()
    if len(catalog.objects) == cnt:
        print >> sys.stderr, "  OK"
    else:
        raise RuntimeError("Could not verify files in catalog")

    if len(catalog.objects) == 0:
        raise RuntimeError("no upgrade files found")

    # write sections file
    catalog.calculateHash()
    muxer = QBUpgradeCatalog.SectionMuxer()
    muxer.addSource(catalog)
    for obj in catalog.objects:
        for f in obj.files:
            muxer.addSource(f)
    if output_file_name == "-":
        print >> sys.stderr, "> writing MPEG TS sections to standard output"
        muxer.write(sys.stdout)
    else:
        file_name = output_file_name
        if os.path.exists(file_name):
            print >> sys.stderr, "> renaming '%s' to '%s~'" % (file_name, file_name)
            if os.path.exists(file_name + "~"):
                os.remove(file_name + "~")
            os.rename(file_name, file_name + "~")
        print >> sys.stderr, "> writing MPEG TS sections to file '%s'" % file_name
        with open(file_name, "w") as output_file:
            muxer.write(output_file)


def main():
    usage = "usage: %prog [ options ] CATALOG_FILE"
    parser = OptionParser(usage=usage)
    parser.add_option("-o", "--output",
                      dest="output_file_name", default="", metavar="FILE",
                      help="write MPEG TS sections to FILE")
    (options, args) = parser.parse_args()
    if len(args) != 1:
        raise RuntimeError("invalid usage, try --help option")

    catalog_file_name = args[0]
    if catalog_file_name != "-" and not os.path.exists(catalog_file_name):
        raise RuntimeError("file '%s' does not exist" % catalog_file_name)

    make_sections(catalog_file_name, options.output_file_name)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
