#!/usr/bin/env python
import QBUpgradeCatalog
from optparse import OptionParser
import traceback
import sys
import os


def scan_upgrade_catalog(output_file_name, directory_path, sbuild_path):

    print >> sys.stderr, "> scanning current directory:"
    catalog = QBUpgradeCatalog.Catalog()
    catalog.scan(directory_path, sbuild_path)
    print >> sys.stderr, "  done, found %d object(s)" % len(catalog.objects)
    catalog.calculateHash()

    if len(catalog.objects) == 0:
        raise RuntimeError("no upgrade files found")

    # write catalog file
    if output_file_name == "-":
        print >> sys.stderr, "> writing catalog to standard output"
        catalog.write(sys.stdout)
    else:
        file_name = output_file_name
        if os.path.exists(file_name):
            print >> sys.stderr, "> renaming '%s' to '%s~'" % (file_name, file_name)
            if os.path.exists(file_name + "~"):
                os.remove(file_name + "~")
            os.rename(file_name, file_name + "~")
        print >> sys.stderr, "> writing catalog to file '%s'" % file_name
        with open(file_name, "w") as output_file:
            catalog.write(output_file)


def main():
    usage = "usage: %prog [options]"
    parser = OptionParser(usage=usage)
    parser.add_option("-o", "--output",
                      dest="output_file_name", default="-", metavar="FILE",
                      help="write catalog to FILE (default: standard output)")
    parser.add_option("-d", "--dir",
                      dest="dir_path",
                      help="path to directory containing the unpacked upgrade")
    parser.add_option("-s", "--sbuild",
                      dest="sbuild_path",
                      help="path to sbuild of the unpacked upgrade")

    (options, args) = parser.parse_args()
    if len(args) != 0 or not options.dir_path or not options.sbuild_path:
        raise RuntimeError("invalid usage, try --help option")

    scan_upgrade_catalog(options.output_file_name, options.dir_path, options.sbuild_path)

if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
