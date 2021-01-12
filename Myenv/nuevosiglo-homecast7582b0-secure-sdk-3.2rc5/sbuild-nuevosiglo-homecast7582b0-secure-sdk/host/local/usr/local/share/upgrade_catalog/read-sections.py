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


def dump_data(data):
    str = ''
    for i, b in enumerate(data):
        str += ' %02x' % ord(b)
        if i % 16 == 15:
            str += '\n'
    print >>sys.stderr, str


versions = dict()
def handle_section(sec, output_file, options):
    global versions
    sec_hdr = struct.unpack(">BH", sec[0:3])
    sec_type = sec_hdr[0]
    sec_len = sec_hdr[1] & 0x0fff

    if options.section_type > 0 and sec_type != options.section_type:
        return

    if sec_hdr[1] & 0x8000 == 0:
        # section_syntax_indicator not set
        print >>sys.stderr, 'new section: type %#x, len %d' % (sec_type, sec_len)
        if options.verbose:
            dump_data(sec)
            print >>sys.stderr, 'writing %d B' % len(sec)
        output_file.write(sec)
        return

    ext_hdr = struct.unpack(">BHHBBB", sec[0:8])
    current_next = ext_hdr[3] & 0x01
    version = (ext_hdr[3] >> 1) & 0x0f
    sec_number = ext_hdr[4]
    last_sec_number = ext_hdr[5]

    if not versions.has_key(sec_type):
        if sec_number == 0:
            versions[sec_type] = (version, 0)
        else:
            if options.verbose:
                print >>sys.stderr, 'not-in-order section: type %#x, version %#x, number %d/%d' \
                        % (sec_type, version, sec_number, last_sec_number)
            return
    else:
        (ver, last_num) = versions[sec_type]
        if version == ver and sec_number <= last_num:
            if options.verbose:
                print >>sys.stderr, 'repeated section: type %#x, version %#x, number %d/%d' \
                        % (sec_type, version, sec_number, last_sec_number)
            return
        elif version == ver and sec_number > last_num + 1:
            if options.verbose:
                print >>sys.stderr, 'not-in-order section: type %#x, version %#x, number %d/%d' \
                        % (sec_type, version, sec_number, last_sec_number)
            return
        else:
            versions[sec_type] = (version, sec_number)

    print >>sys.stderr, 'new section: type %#x, len %d, ID %#x, version %#x (%s), number %d/%d' \
                % (sec_type, sec_len, ext_hdr[2], version, "current" if current_next else "next", sec_number, last_sec_number)

    sec_crc32 = struct.unpack(">L", sec[-4:])[0]
    crc32 = mpeg_crc32(sec[:-4])
    if crc32 != sec_crc32:
        if not options.ignore_crc:
            print >>sys.stderr, "checksums don't match (read 0x%08x, computed 0x%08x), dropping" % (sec_crc32, crc32)
            return
        print >>sys.stderr, "checksums don't match: read 0x%08x, computed 0x%08x" % (sec_crc32, crc32)

    if options.verbose:
        dump_data(sec)
        print >>sys.stderr, 'writing %d B' % len(sec)

    output_file.write(sec)


def process_stream(input_file, output_file, options):
    payload = ''
    has_ptr = False
    last_cc = 0xff
    sec_len = 0
    sec = ''

    while True:
        pkt = input_file.read(188)
        if len(pkt) != 188:
            break

        hdr = struct.unpack(">BHBB", pkt[0:5])
        pid = hdr[1] & 0x1fff
        if hdr[0] != 0x47 or pid != options.PID:
            continue

        # check continuity counter
        cc = hdr[2] & 0x0f
        if cc != (last_cc + 1) % 16:
            # reset state
            payload = ''
            last_cc = cc
            continue
        last_cc = cc

        adaptation_field_present = True if (hdr[2] & 0x20) != 0 else False
        payload_present = True if (hdr[2] & 0x10) != 0 else False
        if not payload_present:
            continue

        if hdr[1] & 0x4000 != 0:
            payload = ''
            has_ptr = True
        elif len(payload) == 0:
            continue

        if not adaptation_field_present:
            payload += pkt[4:]
        else:
            adaptation_field_len = hdr[3]
            if (payload_present and adaptation_field_len > 182) or (not payload_present and adaptation_field_len != 183):
                # drop invalid packet
                payload = ''
                pkt_cnt = 0
                continue
            payload += pkt[5+adaptation_field_len:]

        if has_ptr:
            ptr = struct.unpack(">B", payload[0])[0]
            if ptr + 4 > len(payload):
                continue
            payload = payload[ptr+1:]
            has_ptr = False

        # got all complete sections from payload of consecutive packets
        while len(payload) >= 3:
            sec_hdr = struct.unpack(">BH", payload[0:3])
            sec_len = sec_hdr[1] & 0x0fff
            if sec_len + 3 > len(payload):
                break
            sec = payload[0:sec_len+3]
            handle_section(sec, output_file, options)
            if len(payload) > sec_len + 3:
                first_byte = struct.unpack(">B", payload[sec_len+3:sec_len+4])[0]
                if first_byte != 0xff:
                    payload = payload[sec_len+3:]
                else:
                    payload = ''


def main():
    parser = OptionParser(usage = "usage: %prog [ options ]")
    parser.add_option("-i", "--input",
                      action="store", dest="input_file_name", default="-",
                      metavar="FILE", help="read MPEG TS from FILE (default: standard input)")
    parser.add_option("--ignore-crc",
                      action="store_true", dest="ignore_crc",
                      help="accept sections with invalid CRC32 checksums")
    parser.add_option("-o", "--output",
                      action="store", dest="output_file_name", default="-",
                      metavar="FILE", help="write MPEG TS sections to FILE (default: standard output)")
    parser.add_option("-P", "--pid",
                      action="store", type="int", dest="PID", default="0",
                      metavar="PID", help="MPEG TS PID to read sections from (default: 0)")
    parser.add_option("-t", "--type",
                      action="store", type="int", dest="section_type", default="-1",
                      metavar="TYPE", help="read only sections of specified TYPE (default: any)")
    parser.add_option("-v", "--verbose",
                      action="store_true", dest="verbose",
                      help="output more diagnostic messages")

    (options, args) = parser.parse_args()

    if options.PID < 0 or options.PID >= 0x1fff:
        print >>sys.stderr, "read-sections.py: invalid PID passed"
        return 1

    if options.section_type < -1 or options.section_type > 0xff:
        print >>sys.stderr, "read-sections.py: invalid section type passed"
        return 1

    try:
        if options.input_file_name == '-':
            input_file = sys.stdin
        else:
            input_file = open(options.input_file_name, 'r')
    except Exception, ex:
        print >>sys.stderr, "read-sections.py: can't open file '%s' for reading" % options.input_file_name
        return 1

    try:
        if options.output_file_name == '-':
            output_file = sys.stdout
        else:
            output_file = open(options.output_file_name, 'w')
    except Exception, ex:
        print >>sys.stderr, "read-sections.py: can't open file '%s' for writing" % options.output_file_name
        return 1

    try:
        process_stream(input_file, output_file, options)
    except Exception, ex:
        raise
        print >>sys.stderr, "read-sections.py: %s" % str(ex)
        return 1
    finally:
        input_file.close()
        output_file.close()

    return 0


if __name__ == '__main__':
    sys.exit(main())
