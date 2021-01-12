#!/usr/bin/env python
import sys, os
import struct
import ctypes
from optparse import OptionParser


def mpeg_crc32(data):
    crc = ctypes.c_uint(0xffffffff)
    poly = ctypes.c_uint(0x04c11db7)
    for d in data:
        v = ctypes.c_uint(ord(d))
        for i in range(0, 8):
            x = ctypes.c_uint((crc.value >> 24) ^ v.value)
            if (x.value & 0x80) != 0:
                crc.value = (crc.value << 1) ^ poly.value
            else:
                crc.value = crc.value << 1
            v.value = v.value << 1
    return crc.value & 0xffffffff


cc = 0
def handle_section(header, payload, output_file, options):
    global cc

    sec_hdr = struct.unpack(">BH", header)
    payload_len = sec_hdr[1] & 0x0fff
    pkt_cnt = 0

    # write first TS packet
    pkt = struct.pack(">BHBBBH", 0x47, 0x4000 | options.PID, 0x10 + cc, 0, sec_hdr[0], sec_hdr[1])
    cc = (cc + 1) % 16
    if payload_len >= 180:
        pkt += payload[0:180]
    else:
        pkt += payload
        pkt += '\xff' * (180 - payload_len)
    output_file.write(pkt)
    pkt_cnt += 1
    payload_len -= 180
    offs = 180

    # write consecutive TS packets
    while payload_len > 0:
        pkt = struct.pack(">BHB", 0x47, options.PID, 0x10 + cc)
        cc = (cc + 1) % 16
        if payload_len >= 184:
            pkt += payload[offs:offs+184]
        else:
            pkt += payload[offs:]
            pkt += '\xff' * (184 - payload_len)
        output_file.write(pkt)
        pkt_cnt += 1
        offs += 184
        payload_len -= 184

    return pkt_cnt


def process_sections(input_file, output_file, options):
    if options.repeat_cnt > 1:
        sections = list()
    pkt_cnt = 0

    header = input_file.read(3)
    while len(header) == 3:
        sec_hdr = struct.unpack(">BH", header)
        payload_len = sec_hdr[1] & 0x0fff
        payload = input_file.read(payload_len)
        if len(payload) != payload_len:
            break

        if sec_hdr[1] & 0x4000 != 0:
            # check CRC32
            sec_crc32 = struct.unpack(">L", payload[-4:])[0]
            crc32 = mpeg_crc32(header + payload[:-4])
            if sec_crc32 != crc32:
                msg = "checksums don't match (read 0x%08x, computed 0x%08x)," % (sec_crc32, crc32)
                if options.fix_crc:
                    print >>sys.stderr, msg, 'fixing'
                    payload = payload[:-4] + struct.pack(">L", crc32)
                else:
                    print >>sys.stderr, msg, 'dropping section'
                    header = input_file.read(3)
                    continue

        if options.repeat_cnt > 1:
            sections.append((header, payload))

        pkt_cnt += handle_section(header, payload, output_file, options)

        # read next section header
        header = input_file.read(3)

    while options.repeat_cnt > 1:
        options.repeat_cnt -= 1
        for header, payload in sections:
            pkt_cnt += handle_section(header, payload, output_file, options)

    print >>sys.stderr, "written %d TS packets" % pkt_cnt


def main():
    parser = OptionParser(usage = "usage: %prog [ options ]")
    parser.add_option("-i", "--input",
                      action="store", dest="input_file_name", default="-",
                      metavar="FILE", help="read MPEG TS sections from FILE (default: standard input)")
    parser.add_option("--fix-crc",
                      action="store_true", dest="fix_crc",
                      help="fix invalid CRC32 checksums")
    parser.add_option("-o", "--output",
                      action="store", dest="output_file_name", default="-",
                      metavar="FILE", help="write MPEG TS to FILE (default: standard output)")
    parser.add_option("-P", "--pid",
                      action="store", type="int", dest="PID", default="0",
                      metavar="PID", help="MPEG TS PID to write sections to (default: 0)")
    parser.add_option("--repeat",
                      action="store", type="int", dest="repeat_cnt", default="1",
                      metavar="COUNT", help="repeat output COUNT times (default: 1)")
    parser.add_option("-v", "--verbose",
                      action="store_true", dest="verbose",
                      help="output more diagnostic messages")

    (options, args) = parser.parse_args()

    if options.PID < 0 or options.PID >= 0x1fff:
        print >>sys.stderr, "write-sections.py: invalid PID passed"
        return 1

    if options.repeat_cnt <= 0:
        print >>sys.stderr, "write-sections.py: invalid repeat count passed"
        return 1

    try:
        if options.input_file_name == '-':
            input_file = sys.stdin
        else:
            input_file = open(options.input_file_name, 'r')
    except Exception, ex:
        print >>sys.stderr, "write-sections.py: can't open file '%s' for reading" % options.input_file_name
        return 1

    try:
        if options.output_file_name == '-':
            output_file = sys.stdout
        else:
            output_file = open(options.output_file_name, 'w')
    except Exception, ex:
        print >>sys.stderr, "write-sections.py: can't open file '%s' for writing" % options.output_file_name
        return 1

    try:
        process_sections(input_file, output_file, options)
    except Exception, ex:
        raise
        print >>sys.stderr, "write-sections.py: %s" % str(ex)
        return 1
    finally:
        input_file.close()
        output_file.close()

    return 0


if __name__ == '__main__':
    sys.exit(main())
