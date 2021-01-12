#!/usr/bin/env python
import ota_config
import QBUpgradeCatalog
import TSMux
import DVB

from optparse import OptionParser
import struct
import traceback
import sys
import os

CUBIWARE_OUI = "\x51\x42\x57"

class StreamIdentifierDescriptor(TSMux.Descriptor):
    def __init__(self, component_tag):
        data = struct.pack(">B", component_tag)
        TSMux.Descriptor.__init__(self, 0x52, data)


class DataBroadcastIDDescriptor(TSMux.Descriptor):
    def __init__(self, data_broadcast_id):
        data = struct.pack(">HB", data_broadcast_id, 0)
        TSMux.Descriptor.__init__(self, 0x66, data)
        self._data_broadcast_id = data_broadcast_id
        self._items = list()

    class _Item(object):
        def __init__(self, oui, update_type, update_version=-1, selector=''):
            if len(oui) != 3:
                raise RuntimeError('invalid OUI passed')
            if update_type < 0 or update_type > 3:
                raise RuntimeError('invalid update type %d passed' % update_type)
            if update_version < -1 or update_version > 31:
                raise RuntimeError('invalid update version %d passed' % update_version)
            self.oui = oui
            self.update_type = update_type
            self.update_version = update_version
            self.selector = selector

    def generate_ssu_data(self):
        ssu = ''
        self.length = 3
        for i in self._items:
            self.length += 6 + len(i.selector)
        ssu = struct.pack(">B", self.length - 3)
        for i in self._items:
            if i.update_version >= 0:
                version = 0x20 | i.update_version
            else:
                version = 0
            ssu += i.oui + struct.pack(">BBB", 0xf0 | i.update_type, 0xc0 | version, len(i.selector)) + i.selector
        return ssu

    def _generate_data(self):
        self.data = struct.pack(">H", self._data_broadcast_id)
        self.data += self.generate_ssu_data()

    def add_item(self, oui, update_type, update_version=-1, selector=None):
        self._items.append(self._Item(oui, update_type, update_version, selector))
        self._generate_data()

def create_Cubiware_SSU_DataBroadcastIDDescriptor(obj):
    desc = DataBroadcastIDDescriptor(0x000a)
    desc.add_item(CUBIWARE_OUI, 0, selector=obj.getSelector())
    return desc

def create_Cubiware_SSU_LinkageDescriptor(obj, sid, data, dl_sys):
    onid = data[dl_sys]["ONID"]
    tsid = data[dl_sys]["TSID"]
    transport_stream_pair = (int(tsid), int(onid))
    private_data = '\x0c' + CUBIWARE_OUI + '\x08' + obj.getSelector()
    desc = DVB.LinkageDescriptor(transport_stream_pair, sid, 0x09, private_data)
    return desc

def generate_ssu_link(obj, sid, data, dl_sys):
    desc = create_Cubiware_SSU_LinkageDescriptor(obj, sid, data, dl_sys)
    return desc.getLink()

def generate_ssu_info(obj):
    desc = create_Cubiware_SSU_DataBroadcastIDDescriptor(obj)
    return desc.generate_ssu_data()

def generate_ts_tables(muxer, sid, carousel_pid, obj, pmt_pid, data, dl_sys):
    pmt = TSMux.PMT(sid)
    pmt.addStream(streamType=0x0b, PID=carousel_pid)

    sel = obj.getSelector()
    hex_selector = ''
    for b in sel:
        hex_selector += '%02x' % ord(b)
    print >> sys.stderr, "  using selector '%s' for object %s" % (hex_selector, obj)
    desc = create_Cubiware_SSU_DataBroadcastIDDescriptor(obj)
    pmt.addDescriptor(carousel_pid, desc)

    desc = StreamIdentifierDescriptor(0x90)
    pmt.addDescriptor(carousel_pid, desc)

    pmt_input = TSMux.TSInput(pmt, PID=pmt_pid)
    muxer.addInput(pmt_input)

    tsid = data[dl_sys]["TSID"]
    onid = data[dl_sys]["ONID"]

    if not (0 < tsid < 0xffff):
        raise RuntimeError("invalid tsid passed")

    if not (0 < onid < 0xffff):
        raise RuntimeError("invalid onid passed")

    pat = TSMux.PAT(TSID=int(tsid))
    pat.addProgram(sid, pmt_pid)
    pat_input = TSMux.TSInput(pat, PID=0)
    muxer.addInput(pat_input)

    # watch TSID/ONID parameters when generating satellite OTA streams.
    # They have to match TSID/ONID from /etc/vod/ota-muxes on the STB!

    # It should be disabled for commercial releases (network operator assigns its own NIT table)
    dl_sys_short = ota_config.upgrade_stream_types[dl_sys]

    transport_stream_pair = (int(tsid), int(onid))
    nit = DVB.NIT(NID=1)
    nit.addTransportStream(transport_stream_pair)
    if dl_sys_short == "ter":
        constellation_dictionary = {"QPSK": 0,
                                    "QAM16": 1,
                                    "QAM64": 2}
        desc = DVB.TerrestrialDeliverySystemDescriptor(frequency=data[dl_sys]["frequency"] * 1000,
                                                       bandwidth=data[dl_sys]["bandwidth"],
                                                       constellation=constellation_dictionary[data[dl_sys]["constellation"]],
                                                       guard_interval=2, other_freq_flag=0)
    elif dl_sys_short == "sat":
        modulation_dictionary = {"auto": 0,
                                 "QPSK": 1,
                                 "8PSK": 2,
                                 "QAM16": 3}
        position = float(data[dl_sys]["orbital_position"][:-1])
        position_flag = data[dl_sys]["orbital_position"][-1]
        polarisation = data[dl_sys]["polarization"]
        desc = DVB.SatelliteDeliverySystemDescriptor(frequency=data[dl_sys]["frequency"] * 1000,
                                                     position=int(position * 10),
                                                     west_east_flag=(0 if position_flag == "W" else 1),
                                                     polarisation=(0 if polarisation == "horizontal" else 1),
                                                     modulation=modulation_dictionary[data[dl_sys]["modulation"]],
                                                     symbolRate=data[dl_sys]["symbol_rate"] * 1000)
    else:
        modulation_dictionary = {"undefined": 0,
                                 "QAM16": 1,
                                 "QAM32": 2,
                                 "QAM64": 3,
                                 "QAM128": 4,
                                 "QAM256": 5}
        desc = DVB.CableDeliverySystemDescriptor(frequency=data[dl_sys]["frequency"] * 1000,
                                                 modulation=modulation_dictionary[data[dl_sys]["modulation"]],
                                                 symbolRate=data[dl_sys]["symbol_rate"] * 1000)

    print >> sys.stderr, "  generating upgrade for deliverySystem: %s, parameters: %s" % (dl_sys_short, data[dl_sys])
    nit.addDescriptor(transport_stream_pair, desc)
    desc = create_Cubiware_SSU_LinkageDescriptor(obj, sid, data, dl_sys)
    nit.addDescriptor(-1, desc)
    nit_input = TSMux.TSInput(nit, PID=0x10)
    muxer.addInput(nit_input)


def generate_packets_and_write_to_output(output_file, sections_file_name, sid, carousel_pid,
                                         obj, pmt_pid, data, dl_sys):
    # prepare TS multiplexer
    muxer = TSMux.TSMuxer()

    sec_stream = TSMux.PrivateSectionStream(sections_file_name)
    sec_stream_input = TSMux.TSInput(sec_stream, PID=carousel_pid)
    muxer.addInput(sec_stream_input)

    add_tables = data["general"]["generate_system_information"]
    if add_tables:
        generate_ts_tables(muxer, sid, carousel_pid, obj, pmt_pid, data, dl_sys)

    # write MPEG TS file
    pkt = muxer.getNextPacket()
    pkt_cnt = 0
    while pkt:
        output_file.write(pkt)
        pkt_cnt += 1
        pkt = muxer.getNextPacket()
    print >> sys.stderr, "> done, written %d TS packets" % pkt_cnt


def generate_info_and_write_to_output(output_file, sid, obj, data, dl_sys):
    hex_private_data = ''
    for b in generate_ssu_info(obj):
        hex_private_data += '%02x' % ord(b)
    output_file.write("ssu_info (PMT): " + hex_private_data + "\r\n")

    hex_private_data = ''
    for b in generate_ssu_link(obj, sid, data, dl_sys):
        hex_private_data += '%02x' % ord(b)
    output_file.write("ssu_link (NIT): " + hex_private_data + "\r\n")


def sections_to_ts(catalog_file_name, sections_file_name, build_dir_path, output_file_name, dl_sys, data):
    sid = data["general"]["SID"]
    pmt_pid = data["general"]["PMT_PID"]
    carousel_pid = data["general"]["carousel_PID"]
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

    if len(catalog.objects) != 1:
        raise RuntimeError("catalog has multiple objects")

    obj = catalog.objects[0]
    if not build_dir_path:
        raise RuntimeError("sbuild for %s not found" % obj)
    elif not obj.verifyBuildDir(build_dir_path):
        raise RuntimeError("invalid sbuild '%s'" % build_dir_path)

    if sections_file_name != "-" and not os.path.exists(sections_file_name):
        raise RuntimeError("file '%s' does not exist" % sections_file_name)

    if not (0 < sid < 0xffff):
        raise RuntimeError("invalid sid passed")

    if not (0 < pmt_pid < 0x1fff):
        raise RuntimeError("invalid PMT PID passed")

    if (not (0 < carousel_pid < 0x1fff)) or carousel_pid == pmt_pid:
        raise RuntimeError("invalid carousel PID passed")

    # prepare output files
    if output_file_name == "-":
        print >> sys.stderr, "> writing MPEG TS stream to standard output"
        output_file = sys.stdout
        generate_packets_and_write_to_output(output_file, sections_file_name,
                                             sid, carousel_pid, obj, pmt_pid, data, dl_sys)
        generate_info_and_write_to_output(output_file, sid, obj, data, dl_sys)
    else:
        file_name = output_file_name
        info_file_name = output_file_name + ".info"
        if os.path.exists(file_name):
            print >> sys.stderr, "> renaming '%s' to '%s~'" % (file_name, file_name)
            if os.path.exists(file_name + "~"):
                os.remove(file_name + "~")
            os.rename(file_name, file_name + "~")
        print >> sys.stderr, "> writing catalog to file '%s'" % file_name
        with open(file_name, "w") as output_file:
            generate_packets_and_write_to_output(output_file, sections_file_name,
                                                 sid, carousel_pid, obj, pmt_pid, data, dl_sys)
        with open(info_file_name, "w") as info_output_file:
            generate_info_and_write_to_output(info_output_file, sid, obj, data, dl_sys)


def main():
    usage = "usage: %prog [ options ] CATALOG_FILE SECTIONS_FILE SBUILD_PATH ..."
    parser = OptionParser(usage=usage)
    parser.add_option("-o", "--output", action="store", dest="output_file_name", default="", metavar="FILE",
                      help="write MPEG TS stream to FILE (default: standard output)")
    parser.add_option("-c", "--config", action="store", dest="config_file_path", metavar="FILE",
                      help="path to OTA.json config file")

    (options, args) = parser.parse_args()
    if len(args) < 3:
        raise RuntimeError("invalid usage, try --help option")

    # check if JSON file exists and retrieve contents
    success, data = ota_config.open_config(os.path.abspath(options.config_file_path))
    if not success:
        raise RuntimeError("No JSON config file, aborting process")

    catalog_file_name = args[0]
    if catalog_file_name != "-" and not os.path.exists(catalog_file_name):
        raise RuntimeError("file '%s' does not exist" % catalog_file_name)

    for field in data:
        if field in ota_config.upgrade_stream_types:
            sections_to_ts(catalog_file_name, args[1], args[2], options.output_file_name, field, data)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
