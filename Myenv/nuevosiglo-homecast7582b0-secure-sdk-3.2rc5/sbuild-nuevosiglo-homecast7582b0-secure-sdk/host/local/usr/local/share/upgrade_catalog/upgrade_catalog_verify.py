#!/usr/bin/env python
import sys, os, os.path
from optparse import OptionParser
import QBUpgradeCatalog


def main():
    usage = "usage: %prog [ options ] CATALOG_FILE"
    parser = OptionParser(usage = usage)
    parser.add_option("-o", "--output",
                      dest="outputFileName", default="", metavar="FILE",
                      help="write catalog to FILE")
    (options, args) = parser.parse_args()
    if len(args) != 1:
        print >>sys.stderr, "upgrade_catalog_verify.py: invalid usage, try --help option"
        return 1

    fileName = args[0]
    if fileName != "-" and not os.path.exists(fileName):
        print >>sys.stderr, "upgrade_catalog_verify.py: file '%s' does not exist" % fileName
        return 1

    # try to load existing catalog file
    catalog = QBUpgradeCatalog.Catalog()
    try:
        if fileName == "-":
            print >>sys.stderr, "> loading catalog from standard input"
            catalog.read(sys.stdin)
        else:
            print >>sys.stderr, "> loading catalog from file '%s'" % fileName
            f = open(fileName, "r")
            catalog.read(f)
            f.close()
        print >>sys.stderr, "  done, found %d object(s)" % len(catalog.objects)
        print >>sys.stderr, "> verifying:"
        catalog.verify()
        print >>sys.stderr, "  done, %d valid object(s)" % len(catalog.objects)
    except KeyboardInterrupt:
        print >>sys.stderr, "  TERMINATED"
        return 0
    except Exception, ex:
        print >>sys.stderr, "  FAILED:", str(ex)
        return 1

    if len(catalog.objects) > 0 and options.outputFileName:
        catalog.calculateHash()
        # write catalog file
        if options.outputFileName == "-":
            print >>sys.stderr, "> writing catalog to standard output"
            catalog.write(sys.stdout)
        else:
            fileName = options.outputFileName
            if os.path.exists(fileName):
                print >>sys.stderr, "> renaming '%s' to '%s~'" % (fileName, fileName)
                if os.path.exists(fileName + "~"):
                    os.remove(fileName + "~")
                os.rename(fileName, fileName + "~")
            print >>sys.stderr, "> writing catalog to file '%s'" % fileName
            outputF = open(fileName, "w")
            catalog.write(outputF)
            outputF.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
