#!/usr/bin/env python
import sys, os
import struct
import ctypes


PSIRepeatRate = 64 * 1024


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
    return crc.value


class Descriptor(object):
    def __init__(self, tag, data):
        self.tag = tag
        self.length = len(data)
        self.data = data


class SectionStream(object):
    def __init__(self, repeatRate = 0):
        self.repeatRate = repeatRate
        self.defaultPID = -1
        self.EOS = False

    def reset(self):
        self.EOS = False

    def getNextSection(self):
        return None


class PAT(SectionStream):
    def __init__(self, TSID):
        global PSIRepeatRate
        if TSID <= 0 or TSID >= 0xffff:
            raise Exception("invalid TSID passed")
        SectionStream.__init__(self, PSIRepeatRate)
        self.defaultPID = 0
        self._TSID = TSID
        self._NIT_PID = 0
        self._version = 0
        self.programs = dict()

    def setNIT(self, NIT_PID):
        if NIT_PID <= 0 or NIT_PID >= 0x1fff:
            raise Exception("invalid NIT PID passed")
        self._NIT_PID = NIT_PID

    def addProgram(self, program, PMT_PID):
        if program <= 0 or program >= 0xffff:
            raise Exception("invalid program number %#x passed" % program)
        if PMT_PID <= 0 or PMT_PID >= 0x1fff:
            raise Exception("invalid PMT PID passed for program %#x" % program)
        self.programs[program] = PMT_PID & 0x1fff

    def _constructSection(self):
        l = 12 + 4 * len(self.programs)
        if self._NIT_PID != 0:
            l += 4
        section = struct.pack(">BHHBBB", 0, 0xb000 | (l - 3), self._TSID, 0xc1 | (self._version << 1), 0, 0)
        if self._NIT_PID != 0:
            section += struct.pack(">HH", 0, 0xe000 | self._NIT_PID)
        for program, PMT_PID in self.programs.iteritems():
            section += struct.pack(">HH", program, 0xe000 | PMT_PID)
        section += struct.pack(">L", mpeg_crc32(section))
        return section

    def getNextSection(self):
        if not self.EOS:
            self.EOS = True
            return self._constructSection()
        return None


class PMT(SectionStream):
    def __init__(self, program):
        global PSIRepeatRate
        if program <= 0 or program >= 0xffff:
            raise Exception("invalid program number passed")
        SectionStream.__init__(self, PSIRepeatRate)
        self._program = program
        self._PCR_PID = 0x1fff
        self._version = 0
        self.streams = dict()
        self.descriptors = list()

    def setPCR(self, PCR_PID):
        if PCR_PID <= 0 or PCR_PID >= 0x1fff:
            raise Exception("invalid PCR PID passed")
        self._PCR_PID = PCR_PID

    class _ElementaryStream(object):
        def __init__(self, streamType, PID):
            self.streamType = streamType
            self.PID = PID
            self.descriptors = list()
            self.infoLength = 0

        def addDescriptor(self, descriptor):
            self.descriptors.append(descriptor)
            self.infoLength += 2 + descriptor.length

        def getLength(self):
            return 5 + self.infoLength

    def addStream(self, streamType, PID):
        if streamType <= 0 or streamType > 0xff:
            raise Exception("invalid elementary stream type %#x passed" % streamType)
        if PID <= 0 or PID >= 0x1fff:
            raise Exception("invalid elementary stream PID %#x passed" % PID)
        stream = self._ElementaryStream(streamType, PID)
        self.streams[PID] = stream

    def addDescriptor(self, PID, descriptor):
        if PID <= 0:
            self.descriptors.append(descriptor)
        else:
            self.streams[PID].addDescriptor(descriptor)

    def _constructSection(self):
        programInfoLength = 0
        for desc in self.descriptors:
            programInfoLength += 2 + desc.length
        l = 16 + programInfoLength
        for stream in self.streams.itervalues():
            l += stream.getLength()
        section = struct.pack(">BHHBBBHH", 2, 0xb000 | (l - 3), self._program, 0xc1 | (self._version << 1), 0, 0, 0xe000 | self._PCR_PID, 0xf000 | programInfoLength)
        for desc in self.descriptors:
            section += struct.pack(">BB", desc.tag, desc.length)
            section += desc.data
        for stream in self.streams.itervalues():
            section += struct.pack(">BHH", stream.streamType, 0xe000 | stream.PID, 0xf000 | stream.infoLength)
            for desc in stream.descriptors:
                section += struct.pack(">BB", desc.tag, desc.length)
                section += desc.data
        section += struct.pack(">L", mpeg_crc32(section))
        return section

    def getNextSection(self):
        if not self.EOS:
            self.EOS = True
            return self._constructSection()
        return None


class PrivateSectionStream(SectionStream):
    def __init__(self, filePath):
        SectionStream.__init__(self, 0)
        self._EOS = False
        if filePath == '-':
            self._f = sys.stdin
        else:
            self._f = open(filePath, 'r')

    def getNextSection(self):
        if self._EOS:
            return None
        rawHdr = self._f.read(3)
        if len(rawHdr) != 3:
            # end of stream
            self._f.close()
            self._EOS = True
            return None
        hdr = struct.unpack(">BH", rawHdr)
        l = hdr[1] & 0x0fff
        payload = self._f.read(l)
        if len(payload) == l:
            return rawHdr + payload
        # invalid private sections file
        self._f.close()
        self._EOS = True
        raise Exception("malformed MPEG-TS sections file")


class TSInput(object):
    def __init__(self, stream, PID = -1):
        if PID >= 0:
            self._PID = PID
        else:
            self._PID = stream.defaultPID
        if self._PID < 0:
            raise Exception("no PID passed")
        self._stream = stream
        self._packetsLeft = 0
        self._packetBuffer = list()
        self._cc = 0
        self.packetsCount = 0
        self.packetsTotal = stream.repeatRate / 184
        self._fillBuffer()

    def _wrapSection(self, section):
        hdr = struct.unpack(">BH", section[0:3])
        sectionLen = len(section)
        payloadLen = hdr[1] & 0x0fff
        if sectionLen != payloadLen + 3:
            raise Exception("invalid MPEG-TS section")
        offs = 3
        # create first TS packet
        pkt = struct.pack(">BHBBBH", 0x47, 0x4000 | self._PID, 0x10 + self._cc, 0, hdr[0], hdr[1])
        self._cc = (self._cc + 1) % 16
        if payloadLen >= 180:
            pkt += section[offs:offs+180]
        else:
            pkt += section[offs:sectionLen]
            pkt += '\xff' * (180 - payloadLen)
        self._packetBuffer.append(pkt)
        offs += 180
        payloadLen -= 180
        # create consecutive TS packets
        while offs < sectionLen:
            pkt = struct.pack(">BHB", 0x47, self._PID, 0x10 + self._cc)
            self._cc = (self._cc + 1) % 16
            if payloadLen >= 184:
                pkt += section[offs:offs+184]
            else:
                pkt += section[offs:sectionLen]
                pkt += '\xff' * (184 - payloadLen)
            self._packetBuffer.append(pkt)
            offs += 184
            payloadLen -= 184

    def _fillBuffer(self):
        if self._stream.repeatRate == 0:
            # sections of data stream
            self.packetsCount = 0
            self._packetBuffer = list()
            section = self._stream.getNextSection()
            if section:
                self._wrapSection(section)
            self._packetsLeft = len(self._packetBuffer)
        else:
            # PSI sections
            self._stream.reset()
            self._packetBuffer = list()
            section = self._stream.getNextSection()
            while section:
                self._wrapSection(section)
                section = self._stream.getNextSection()
            self.packetsCount = self._packetsLeft = len(self._packetBuffer)

    def reset(self):
        if self._packetsLeft == 0 and self._stream.repeatRate != 0:
            self._fillBuffer()

    def getNextPacket(self):
        if self._packetsLeft == 0:
            if self._stream.repeatRate == 0:
                self._fillBuffer()
                if self._packetsLeft == 0:
                    # end of stream
                    return None
            else:
                # end of table
                return None
        pkt = self._packetBuffer[len(self._packetBuffer) - self._packetsLeft]
        self._packetsLeft -= 1
        return pkt


class TSMuxer(object):
    def __init__(self):
        self._dataInputs = list()
        self._PSIinputs = list()
        self._activePSIInputsCount = 0
        self._nextPSIPacket = -1
        self._packetsCount = 0
        self._EOS = False

    class _InputDesc(object):
        def __init__(self, input):
            self.input = input
            self.nextPacket = 0
            self.active = True

    def addInput(self, input):
        desc = self._InputDesc(input)
        if input.packetsTotal == 0:
            self._dataInputs.append(desc)
        else:
            desc.nextPacket = 0
            self._PSIinputs.append(desc)
            self._PSIinputs.sort(key = lambda x: x.nextPacket)
            self._nextPSIPacket = self._PSIinputs[0].nextPacket
            self._activePSIInputsCount += 1

    def _getNextPSIPacket(self):
        desc = self._PSIinputs[0]
        pkt = desc.input.getNextPacket()
        if pkt:
            desc.nextPacket += 1
            self._nextPSIPacket = desc.nextPacket
            return pkt
        # finished entire table
        desc.input.reset()
        desc.nextPacket += desc.input.packetsTotal
        self._PSIinputs.sort(key = lambda x: x.nextPacket)
        self._nextPSIPacket = self._PSIinputs[0].nextPacket
        if desc.active:
            desc.active = False
            self._activePSIInputsCount -= 1
            if self._activePSIInputsCount == 0 and len(self._dataInputs) == 0:
                self._EOS = True
        return None

    def _getNextDataPacket(self):
        if len(self._dataInputs) == 0:
            return None
        desc = self._dataInputs.pop(0)
        pkt = desc.input.getNextPacket()
        while not pkt and len(self._dataInputs) > 0:
            desc = self._dataInputs.pop(0)
            pkt = desc.input.getNextPacket()
        if pkt:
            self._dataInputs.append(desc)
        elif self._activePSIInputsCount == 0:
            self._EOS = True
        return pkt

    def _getNullPacket(self):
        return '\x47\x1f\xff\x10' + ('\0' * 184)

    def getNextPacket(self):
        if self._EOS:
            return None
        pkt = None
        if self._nextPSIPacket >= 0 and self._nextPSIPacket <= self._packetsCount:
            # try PSI packet first
            pkt = self._getNextPSIPacket()
        if not pkt:
            # try data packet
            pkt = self._getNextDataPacket()
        if not pkt and not self._EOS:
            pkt = self._getNullPacket()
        if pkt:
            self._packetsCount += 1
        return pkt
