#!/usr/bin/env python
import struct
import TSMux


class NIT(TSMux.SectionStream):
    def __init__(self, NID):
        if NID <= 0 or NID > 0xffff:
            raise Exception("invalid network ID %#x passed" % NID)
        TSMux.SectionStream.__init__(self, TSMux.PSIRepeatRate)
        self._NID = NID
        self._version = 0
        self.transportStreams = list()
        self.descriptors = list()

    class _TransportStream(object):
        def __init__(self, TSID, ONID):
            self.TSID = TSID
            self.ONID = ONID
            self.descriptors = list()
            self.infoLength = 0

        def addDescriptor(self, descriptor):
            self.descriptors.append(descriptor)
            self.infoLength += 2 + descriptor.length

        def getLength(self):
            return 6 + self.infoLength

    def addTransportStream(self, TSID):
        if isinstance(TSID, tuple):
            ONID = TSID[1]
            TSID = TSID[0]
        else:
            ONID = 0
        if TSID < 0 or TSID > 0xffff:
            raise Exception("invalid transport stream ID %#x passed" % TSID)
        if ONID < 0 or ONID >= 0xffff:
            raise Exception("invalid original network ID %#x passed" % ONID)
        for i, stream in enumerate(self.transportStreams):
            if stream.TSID == TSID and stream.ONID == ONID:
                del self.transportStreams[i]
                break
        stream = self._TransportStream(TSID, ONID)
        self.transportStreams.append(stream)

    def addDescriptor(self, TSID, descriptor):
        if isinstance(TSID, int) and TSID < 0:
            self.descriptors.append(descriptor)
        else:
            if isinstance(TSID, tuple):
                ONID = TSID[1]
                TSID = TSID[0]
            else:
                ONID = 0
            for stream in self.transportStreams:
                if stream.TSID == TSID and stream.ONID == ONID:
                    stream.addDescriptor(descriptor)
                    return
            raise Exception("unknown transport stream (%#x, %#x)" % (TSID, ONID))

    def _constructSection(self):
        networkInfoLength = 0
        for desc in self.descriptors:
            networkInfoLength += 2 + desc.length
        l = 16 + networkInfoLength
        for stream in self.transportStreams:
            l += stream.getLength()
        section = struct.pack(">BHHBBBH", 0x40, 0xf000 | (l - 3), self._NID, 0xc1 | (self._version << 1), 0, 0, 0xf000 | networkInfoLength)
        for desc in self.descriptors:
            section += struct.pack(">BB", desc.tag, desc.length)
            section += desc.data
        section += struct.pack(">H", 0xf000 | (l - 16 - networkInfoLength))
        for stream in self.transportStreams:
            section += struct.pack(">HHH", stream.TSID, stream.ONID, 0xf000 | stream.infoLength)
            for desc in stream.descriptors:
                section += struct.pack(">BB", desc.tag, desc.length)
                section += desc.data
        section += struct.pack(">L", TSMux.mpeg_crc32(section))
        return section

    def getNextSection(self):
        if not self.EOS:
            self.EOS = True
            return self._constructSection()
        return None


class LinkageDescriptor(TSMux.Descriptor):
    def __init__(self, TSID, SID, linkageType, privateData = None):
        if isinstance(TSID, tuple):
            ONID = TSID[1]
            TSID = TSID[0]
        else:
            ONID = 0
        if TSID < 0 or TSID > 0xffff:
            raise Exception("invalid transport stream ID %#x passed" % TSID)
        if ONID < 0 or ONID >= 0xffff:
            raise Exception("invalid original network ID %#x passed" % ONID)
        if SID <= 0 or SID >= 0xffff:
            raise Exception("invalid service ID %#x passed" % SID)
        if linkageType <= 0 or linkageType > 0xff:
            raise Exception("invalid linkage type %#x passed" % linkageType)
        data = struct.pack(">HHHB", TSID, ONID, SID, linkageType)
        self.link = privateData
        if self.link:
            data += self.link
        TSMux.Descriptor.__init__(self, 0x4a, data)

    def getLink(self):
        return self.link

def _encodeBCD(value):
    res = 0
    cnt = 0
    while value > 0:
        digit = value % 10
        res = res | (digit << (cnt * 4))
        value = value / 10
        cnt += 1
    return res

class CableDeliverySystemDescriptor(TSMux.Descriptor):
    def __init__(self, frequency, modulation, symbolRate):
        if frequency < 100 * 1000 * 1000 or frequency >= 1000 * 1000 * 1000:
            raise Exception("invalid cable frequency %d passed" % frequency)
        if symbolRate < 1 * 1000 * 1000 or symbolRate >= 100 * 1000 * 1000:
            raise Exception("invalid cable symbolRate %d passed" % symbolRate)
        frequency = _encodeBCD(frequency / 100)
        symbolRate = _encodeBCD(symbolRate / 100)
        data = struct.pack(">LHBL", frequency, 0xfff0, modulation, symbolRate << 4)
        TSMux.Descriptor.__init__(self, 0x44, data)

class TerrestrialDeliverySystemDescriptor(TSMux.Descriptor):
    def __init__(self, frequency, bandwidth, constellation, guard_interval, other_freq_flag):
        if frequency < 100 * 1000 * 1000 or frequency >= 1000 * 1000 * 1000:
            raise Exception("invalid terrestrial frequency %d passed" % frequency)

        frequency = frequency / 10

        data = struct.pack(">LBBBL", frequency, (bandwidth << 5 ) & 0xE0, (constellation << 6) & 0xC0, ((guard_interval << 3) | other_freq_flag) & 0x19, 0xffffffff)
        TSMux.Descriptor.__init__(self, 0x5A, data)

class SatelliteDeliverySystemDescriptor(TSMux.Descriptor):
    def __init__(self, frequency, position, west_east_flag, polarisation, modulation, symbolRate):
        if frequency < 10.7 * 1000 * 1000 * 1000 or frequency >= 12.75 * 1000 * 1000 * 1000:
            raise Exception("invalid satellite frequency %d passed" % frequency)
        if symbolRate < 1 * 1000 * 1000 or symbolRate >= 100 * 1000 * 1000:
            raise Exception("invalid satellite symbolRate %d passed" % symbolRate)
        frequency = _encodeBCD(frequency / 10000)
        position = _encodeBCD(position)
        symbolRate = _encodeBCD(symbolRate / 100)
        data = struct.pack(">LHBL", frequency, position, (((west_east_flag & 0x01) << 7) | ((polarisation & 0x01) << 5) | (modulation & 0x03)) & 0xA3, symbolRate << 4)

        TSMux.Descriptor.__init__(self, 0x43, data)

