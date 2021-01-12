#!/usr/bin/env python
import sys, os, os.path
import xml.dom, xml.dom.minidom
import zlib, gzip
import StringIO
import struct
import subprocess
import re

if sys.version_info < (2, 5):
    import md5
else:
    import hashlib


class SectionSource(object):
    SECTION_TYPE = 0x3e
    SECTION_LENGTH = 21 * 184

    def __init__(self, type, repeatRate=0):
        self.offset = 0
        self.type = type
        self.repeatRate = repeatRate
        self.repeatCount = 0

    def writeNextSection(self, f):
        # calculate number of bytes to dump
        length = self.size - self.offset
        if length > self.SECTION_LENGTH - 12 - 4:
            length = self.SECTION_LENGTH - 12 - 4
        # dump MPEG-TS private section header
        f.write(struct.pack(">BH", self.SECTION_TYPE, (length + 12) | 0x3000))
        # dump our own section header
        f.write(struct.pack("<BBHII", self.type, 0, self.sid, self.offset, self.size))
        self._writeRawData(f, self.offset, length)
        # update counters
        self.offset = self.offset + length
        if self.offset == self.size:
            self.offset = 0
            self.repeatCount = self.repeatCount + 1
        return length


class SectionSourceDescriptor(object):
    def __init__(self, src):
        self.src = src
        self.nextOffset = 0
        self.progress = 0
        self.dynamicPriority = 0.0


class SectionMuxer(object):
    def __init__(self):
        self.sources = list()

    def addSource(self, source):
        self.sources.append(source)

    def write(self, outf):
        # prepare temporary structures
        sources = list()
        vbrSourcesCount = 0
        for src in self.sources:
            descriptor = SectionSourceDescriptor(src)
            if type(src) == Catalog:
                sources.insert(0, descriptor)
            else:
                if src.repeatRate == 0:
                    vbrSourcesCount += 1
                sources.append(descriptor)
        # output
        total = 0
        while vbrSourcesCount > 0:
            for descriptor in sources:
                if descriptor.nextOffset > total:
                    descriptor.dynamicPriority = 1.0 + descriptor.nextOffset - total
                    continue
                l = descriptor.src.writeNextSection(outf)
                total += l
                descriptor.progress += l
                if descriptor.progress == descriptor.src.size:
                    if descriptor.src.repeatRate > 0:
                        descriptor.nextOffset += descriptor.src.repeatRate
                        descriptor.progress = 0
                    else:
                        sources.remove(descriptor)
                        vbrSourcesCount -= 1
                        continue
                if descriptor.src.repeatRate > 0 and descriptor.nextOffset > total:
                    descriptor.dynamicPriority = 1.0 + descriptor.nextOffset - total
                else:
                    descriptor.dynamicPriority = 1.0 * descriptor.progress / descriptor.src.size
            sources.sort(key=lambda descriptor: descriptor.dynamicPriority)
        pass


def _makeTextElement(doc, name, value, attributes=None):
    elem = doc.createElement(name)
    elem.appendChild(doc.createTextNode(value))
    if attributes:
        for name, value in attributes.iteritems():
            elem.setAttribute(name, value)
    return elem


def _findElements(parent, name):
    results = list()
    for node in parent.childNodes:
        if node.nodeType == xml.dom.Node.ELEMENT_NODE and node.nodeName == name:
            results.append(node)
    return results


def _findElement(parent, name, required=False, attributes=None):
    elem = None
    for node in parent.childNodes:
        if node.nodeType == xml.dom.Node.ELEMENT_NODE and node.nodeName == name:
            if not elem:
                if attributes:
                    for attrName, attrValue in attributes.iteritems():
                        if not node.hasAttribute(attrName) or node.getAttribute(attrName) != attrValue:
                            node = None
                            break
                elem = node
            else:
                raise Exception("element %s occurs multiple times" % name)
    if not elem and required:
        raise Exception("missing element %s" % name)
    return elem


def _getElementText(elem):
    if len(elem.childNodes) == 0:
        return ""
    elif len(elem.childNodes) == 1 and elem.childNodes[0].nodeType == xml.dom.Node.TEXT_NODE:
        return elem.childNodes[0].nodeValue.strip()
    raise Exception("invalid %s element" % elem.nodeName)


class CatalogFile(SectionSource):
    QBCAROUSEL_TYPE = 9
    PIECE_LENGTH = 512 * 1024

    def __init__(self, obj, name):
        SectionSource.__init__(self, self.QBCAROUSEL_TYPE)
        self.catalogObject = obj
        self.name = name
        self.path = os.path.join(obj.dirPath, name)
        try:
            st = os.stat(self.path)
            self.size = st.st_size
        except Exception, ex:
            self.size = None
        self.md5sum = None
        self.sid = None
        self.URL = None

    def __repr__(self):
        return self.path

    def load(self, rootElem):
        self.size = long(_getElementText(_findElement(rootElem, "size", True)))
        verificationElem = _findElement(rootElem, "verification", True)
        hashElem = _findElement(verificationElem, "hash", True, {"type": "md5"})
        self.md5sum = _getElementText(hashElem)
        resourcesElem = _findElement(rootElem, "resources", True)
        for node in _findElements(resourcesElem, "resource"):
            type = node.getAttribute("type")
            if type == "qbcarousel":
                self.sid = int(_getElementText(_findElement(node, "sid", True)))
        pass

    def _computeHash(self, f, length=0):
        if sys.version_info < (2, 5):
            digest = md5.new()
        else:
            digest = hashlib.md5()
        done = 0
        while length == 0 or done < length:
            if length == 0 or length - done > 65536:
                block = f.read(65536)
            else:
                block = f.read(length - done)
            digest.update(block)
            done = done + len(block)
            if len(block) < 65536:
                break
        return digest.hexdigest()

    def verify(self):
        if not os.path.exists(self.path):
            print >> sys.stderr, "    file %s does not exist" % self.path
            return False
        st = os.stat(self.path)
        if st.st_size != self.size:
            print >> sys.stderr, "    size of file %s differs" % self.path
            return False
        f = open(self.path, "r")
        md5sum = self._computeHash(f)
        f.close()
        if md5sum != self.md5sum:
            print >> sys.stderr, "    MD5 sum of file %s differs" % self.path
            return False
        return True

    def dump(self, doc, baseURI=None):
        # update internal information
        st = os.stat(self.path)
        self.size = st.st_size
        if self.size < 16 * 1024:
            self.repeatRate = 1024 * 1024
        if not self.md5sum:
            f = open(self.path, "r")
            self.md5sum = self._computeHash(f)
            f.close()
        if not self.sid:
            self.sid = self.catalogObject.catalog.allocSID()
        if not self.URL and baseURI:
            self.URL = self.path
        # create top-level DOM node
        fileElem = doc.createElement("file")
        fileElem.appendChild(_makeTextElement(doc, "name", self.name))
        fileElem.appendChild(_makeTextElement(doc, "size", str(self.size)))
        # add verification nodes
        verificationElem = doc.createElement("verification")
        verificationElem.appendChild(_makeTextElement(doc, "hash", self.md5sum, {"type": "md5"}))
        if self.size > self.PIECE_LENGTH:
            piecesElem = doc.createElement("pieces")
            piecesElem.setAttribute("type", "md5")
            piecesElem.setAttribute("length", str(self.PIECE_LENGTH))
            f = open(self.path, "r")
            for i in range(0, (self.size + self.PIECE_LENGTH - 1) / self.PIECE_LENGTH):
                hashElem = doc.createElement("hash")
                hashElem.setAttribute("piece", str(i))
                hashElem.appendChild(doc.createTextNode(self._computeHash(f, self.PIECE_LENGTH)))
                piecesElem.appendChild(hashElem)
            f.close()
            verificationElem.appendChild(piecesElem)
        fileElem.appendChild(verificationElem)
        # add resource nodes
        resourcesElem = doc.createElement("resources")
        resourceElem = doc.createElement("resource")
        resourceElem.setAttribute("type", "qbcarousel")
        resourceElem.appendChild(_makeTextElement(doc, "sid", str(self.sid)))
        resourcesElem.appendChild(resourceElem)
        if self.URL:
            resourceElem = doc.createElement("resource")
            resourceElem.setAttribute("type", "download")
            resourceElem.appendChild(_makeTextElement(doc, "url", self.URL))
            resourcesElem.appendChild(resourceElem)
        fileElem.appendChild(resourcesElem)
        return fileElem

    def _writeRawData(self, outf, offset, length):
        inf = open(self.path)
        inf.seek(offset)
        data = inf.read(length)
        inf.close()
        outf.write(data)


class CatalogObject(object):
    def __init__(self, catalog, vendor, board, project, version, dirPath):
        self.catalog = catalog
        self.dirPath = dirPath
        self.vendor = vendor
        self.board = board
        self.project = project
        self.version = version
        self.fullName = "-".join((vendor, board, project, version))
        self.forceUpgrade = False
        self.files = list()
        self._selector = None

    def __repr__(self):
        return self.fullName

    def load(self, rootElem):
        forceElem = _findElement(rootElem, "force-upgrade", False)
        if forceElem and _getElementText(forceElem) == "yes":
            self.forceUpgrade = True
        else:
            self.forceUpgrade = False
        elem = _findElement(rootElem, "files", True)
        for node in _findElements(elem, "file"):
            name = _getElementText(_findElement(node, "name", True))
            file = CatalogFile(self, name)
            file.load(node)
            self.files.append(file)
        pass

    def verify(self):
        if not os.path.exists(self.dirPath):
            print >> sys.stderr, "    directory %s does not exist" % self.dirPath
            return False
        for f in self.files:
            if not f.verify():
                return False
        return True

    def _readFile(self, path):
        f = open(path, 'r')
        str = f.read()
        f.close()
        return str.strip()

    def _extractFile(self, archivePath, fileName):
        tar = subprocess.Popen('tar -x -O -f %s %s' % (archivePath, fileName), shell=True, stdout=subprocess.PIPE,
                               stderr=sys.stderr)
        contents = tar.stdout.read()
        tar.terminate()
        return contents

    def _parseSelector(self, filePath):
        selector = {'board.cpu': 0, 'board.model': 0, 'project': 0}
        f = open(filePath, 'r')
        for line in f:
            line = line.strip()
            if len(line) == 0 or line.startswith('#'):
                continue
            tag, value = line.split(':')
            if tag.lower() == 'board.cpu':
                selector['board.cpu'] = int(value, 16)
            elif tag.lower() == 'board.model':
                selector['board.model'] = int(value, 16)
            elif tag.lower() == 'project':
                selector['project'] = int(value, 16)
        f.close()
        for v in selector.itervalues():
            if v <= 0 or v > 255:
                return False
        self._selector = selector
        return True

    def verifyBuildDir(self, buildDir):
        rootDir = os.path.join(buildDir, 'target', 'root', 'etc')
        return self._parseSelector(os.path.join(rootDir, 'SELECTOR'))

    def dump(self, doc, baseURI):
        objectElem = doc.createElement("object")
        objectElem.appendChild(_makeTextElement(doc, "path", self.dirPath))
        objectElem.appendChild(_makeTextElement(doc, "vendor", self.vendor))
        objectElem.appendChild(_makeTextElement(doc, "board", self.board))
        objectElem.appendChild(_makeTextElement(doc, "project", self.project))
        objectElem.appendChild(_makeTextElement(doc, "version", self.version))
        if self.forceUpgrade:
            objectElem.appendChild(_makeTextElement(doc, "force-upgrade", "yes"))
        if len(self.files) > 0:
            filesElem = doc.createElement("files")
            for f in self.files:
                filesElem.appendChild(f.dump(doc, baseURI))
                objectElem.appendChild(filesElem)
        return objectElem

    def getSelector(self):
        if not self._selector:
            return None
        # first two bytes of selector encode version number
        version = (0, 0, 0)
        if self.version != 'CURRENT':
            majorMinor = self.version.split('-')
            if len(majorMinor) == 1:
                patch = 0
            else:
                patch = int(majorMinor[1].split('rc')[0])
            majorMinor = majorMinor[0].split('rc')[0]
            major = int(majorMinor.split('.')[0])
            minor = int(majorMinor.split('.')[1])
            version = (major, minor, patch)
        selector = chr(0x50 + version[0]) + chr(0x3e + version[1])
        # next three bytes of selector encode platform
        selector += chr(self._selector['board.cpu'])
        selector += chr(self._selector['board.model'])
        selector += chr(self._selector['project'])
        # sixth byte encodes patch version and 'upgrade forced' flag
        if not self.forceUpgrade:
            selector += chr(version[2])
        else:
            selector += chr(0x10 | version[2])
        # last two bytes contain checksum
        selector += chr(0xff - (ord(selector[0]) + ord(selector[2]) + ord(selector[4])) % 0x100)
        selector += chr(0xff - (ord(selector[1]) + ord(selector[3]) + ord(selector[5])) % 0x100)

        return selector;


class Catalog(SectionSource):
    QBCAROUSEL_TYPE = 8

    def __init__(self):
        SectionSource.__init__(self, self.QBCAROUSEL_TYPE, 128 * 1024)
        self.baseURI = None
        self.objects = list()
        self.usedSIDs = set()
        self.maxSID = 0
        self.sid = 0
        self.raw = None
        self.size = None
        self.hash = None

    def checkSID(self, sid):
        if sid in self.usedSIDs:
            return False
        if sid > self.maxSID:
            self.maxSID = sid
        return True

    def allocSID(self):
        sid = self.maxSID + 1
        self.maxSID = sid
        self.usedSIDs.add(sid)
        return sid

    def read(self, f):
        doc = xml.dom.minidom.parse(f)
        if doc.documentElement.getAttribute("version") != "1.0":
            raise Exception("unsupported catalog file format")
        elem = _findElement(doc.documentElement, "base-uri")
        if elem:
            self.baseURI = _getElementText(elem)
        elem = _findElement(doc.documentElement, "objects", True)
        for node in _findElements(elem, "object"):
            vendor = _getElementText(_findElement(node, "vendor", True))
            board = _getElementText(_findElement(node, "board", True))
            project = _getElementText(_findElement(node, "project", True))
            version = _getElementText(_findElement(node, "version", True))
            dirPath = _getElementText(_findElement(node, "path", True))
            obj = CatalogObject(self, vendor, board, project, version, dirPath)
            obj.load(node)
            self.objects.append(obj)
        pass

    def verify(self):
        validObjects = list()
        for o in self.objects:
            print >> sys.stderr, "   object", o
            try:
                if o.verify():
                    validObjects.append(o)
            except Exception, ex:
                pass
        self.objects = validObjects

    def _compress(self, doc):
        src = doc.toprettyxml(encoding="utf-8")
        str = StringIO.StringIO()
        f = gzip.GzipFile("", "w", 9, str)
        f.write(src)
        f.close()
        self.raw = str.getvalue()
        self.size = len(self.raw)
        str.close()
        return src

    def calculateHash(self):
        doc = self._dump()
        self._compress(doc)

    def _dump(self):
        impl = xml.dom.minidom.getDOMImplementation()
        doc = impl.createDocument(None, "upgrade-catalog", None)
        doc.documentElement.setAttribute("version", "1.0")
        if self.baseURI:
            baseURIElem = doc.createElement("base-uri")
            baseURIElem.appendChild(doc.createTextNode(self.baseURI))
            doc.documentElement.appendChild(baseURIElem)
        if len(self.objects) > 0:
            objectsElem = doc.createElement("objects")
            for obj in self.objects:
                objectsElem.appendChild(obj.dump(doc, self.baseURI))
            doc.documentElement.appendChild(objectsElem)
        # add hash for easy detection of catalog file changes
        if sys.version_info < (2, 5):
            digest = md5.new()
        else:
            digest = hashlib.md5()
        digest.update(doc.toxml(encoding="utf-8"))
        self.hash = digest.hexdigest()
        hashElem = doc.createElement("hash")
        hashElem.appendChild(doc.createTextNode(self.hash))
        if doc.documentElement.hasChildNodes():
            doc.documentElement.insertBefore(hashElem, doc.documentElement.childNodes[0])
        else:
            doc.documentElement.appendChild(hashElem)
        # return document
        return doc

    def write(self, outf):
        doc = self._dump()
        src = self._compress(doc)
        outf.write(src)

    def _writeRawData(self, outf, offset, length):
        if not self.raw:
            doc = self._dump()
            self._compress(doc)
        if length == self.size:
            outf.write(self.raw)
        else:
            outf.write(self.raw[offset:offset + length])

    def _readFile(self, path):
        f = open(path, 'r')
        str = f.read()
        f.close()
        return str.strip()

    def _scanDir(self, dirName, dirPath, fileNames, sbuildDir):
        parts = dirName.split("-")
        # find the ARCH file to obtain the 'board' attribute
        archPath = os.path.join(sbuildDir, 'target', 'root', 'etc', 'ARCH')
        if not os.path.isfile(archPath):
            return None
        board = self._readFile(archPath);
        # check for version number in MAJOR.MINOR-PATCH format
        if re.match("^[0-9]+(rc[0-9]+)?$", parts[-1]):
            parts = parts[:-2] + ["-".join(parts[-2:])]
        # check if directory name matches expected pattern
        if len(parts) < 4 or not parts[0] or not parts[1]:
            return None
        # skip '_signed' tag generated for custom-signed images
        strippedVersionTag = parts[-1].replace("_signed", "")
        if strippedVersionTag != 'CURRENT' and not re.match("^[0-9]+\.[0-9]+(-[0-9]+)?(rc[0-9]+)?$", strippedVersionTag):
            return None
        project = "-".join(parts[2:-1])
        baseFileName = None
        for f in fileNames:
            if re.match("^[-a-zA-Z0-9_]+-img-[a-zA-Z0-9]+.tgz$", f):
                baseFileName = f
        if baseFileName:
            obj = CatalogObject(self, parts[0], board, project, parts[-1], dirPath)
            for f in fileNames:
                if f == "force" and os.path.isfile(os.path.join(dirPath, f)):
                    obj.forceUpgrade = True
                else:
                    obj.files.append(CatalogFile(obj, f))
            return obj
        return None

    def scan(self, dirPath, sbuildDir):
        dirName = os.path.basename(dirPath)
        fileNames = os.listdir(dirPath)
        obj = self._scanDir(dirName, os.path.normpath(dirPath), fileNames, sbuildDir)
        if obj:
            self.objects.append(obj)
            print >> sys.stderr, "   object", obj
        self.objects.sort(key=lambda obj: obj.fullName)


if __name__ == "__main__":
    print >> sys.stderr, "!!! NOT AN EXECUTABLE !!!"
    sys.exit(1)
