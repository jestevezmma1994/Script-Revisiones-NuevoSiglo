#!/usr/bin/env python
from optparse import OptionParser
import traceback
import sys
import os

import QBUpgradeCatalog


def set_force_flag(catalog_file_name):
    # try to load existing catalog file
    catalog = QBUpgradeCatalog.Catalog()

    if catalog_file_name == "-":
        print >> sys.stderr, "> loading catalog from standard input"
        catalog.read(sys.stdin)
    else:
        print >> sys.stderr, "> loading catalog from file '%s'" % catalog_file_name
        with open(catalog_file_name, "r") as f:
            catalog.read(f)
    print >> sys.stderr, "  done, found %d object(s)" % len(catalog.objects)

    if len(catalog.objects) == 0:
        raise RuntimeError("> no upgrade objects found")
    elif len(catalog.objects) > 1:
        raise RuntimeError("multiple upgrade objects" % catalog_file_name)

    catalog.objects[0].forceUpgrade = True

    # write catalog file
    if catalog_file_name == "-":
        print >>sys.stderr, "> writing catalog to standard output"
        catalog.write(sys.stdout)
    else:
        if os.path.exists(catalog_file_name):
            print >>sys.stderr, "> renaming '%s' to '%s~'" % (catalog_file_name, catalog_file_name)
            if os.path.exists(catalog_file_name + "~"):
                os.remove(catalog_file_name + "~")
            os.rename(catalog_file_name, catalog_file_name + "~")
        print >>sys.stderr, "> writing catalog to file '%s'" % catalog_file_name
        with open(catalog_file_name, "w") as output_file:
            catalog.write(output_file)


def main():
    usage = "usage: %prog CATALOG_FILE"
    parser = OptionParser(usage=usage)
    (options, args) = parser.parse_args()
    if len(args) != 1:
        raise RuntimeError("invalid usage, try --help option")

    catalog_file_name = args[0]
    if catalog_file_name != "-" and not os.path.exists(catalog_file_name):
        raise RuntimeError("file '%s' does not exist" % catalog_file_name)

    set_force_flag(catalog_file_name)

if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
