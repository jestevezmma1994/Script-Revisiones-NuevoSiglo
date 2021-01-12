#!/usr/bin/env python3

#############################################################################
# Cubiware Sp. z o.o. Software License Version 1.0
#
# Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
#
# Any rights which are not expressly granted in this License are entirely and
# exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
# modify, translate, reverse engineer, decompile, disassemble, or create
# derivative works based on this Software. You may not make access to this
# Software available to others in connection with a service bureau,
# application service provider, or similar business, or make any other use of
# this Software without express written permission from Cubiware Sp. z o.o.
#
# Any User wishing to make use of this Software must contact
# Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
# includes, but is not limited to:
# (1) integrating or incorporating all or part of the code into a product for
#     sale or license by, or on behalf of, User to third parties;
# (2) distribution of the binary or source code to third parties for use with
#     a commercial product sold or licensed by, or on behalf of, User.
#############################################################################

import sys
import os
import argparse
import binascii
import tempfile


chunk_types = {
    b'bKGD': 'Background color',
    b'cHRM': 'Primary chromaticities',
    b'fRAc': 'Fractal parameters (extension)',
    b'gAMA': 'Image gamma',
    b'gIFg': 'GIF conversion info (extension)',
    b'gIFx': 'GIF conversion info (extension)',
    b'hIST': 'Palette histogram',
    b'iCCP': 'Embedded ICC profile',
    b'IDAT': 'Image data',
    b'IEND': 'Image trailer',
    b'IHDR': 'Image header',
    b'iTXt': 'International textual data',
    b'oFFs': 'Image offset (extension)',
    b'pCAL': 'Pixel calibration (extension)',
    b'pHYs': 'Physical pixel dimensions',
    b'PLTE': 'Palette',
    b'sBIT': 'Significant bits',
    b'sCAL': 'Physical scale (extension)',
    b'sRGB': 'Standard RGB color space',
    b'sPLT': 'Suggested palette',
    b'tEXt': 'Textual data',
    b'tIME': 'Image last-modification time',
    b'tRNS': 'Transparency',
    b'vpAg': 'Virtual page (unofficial)',
    b'zTXt': 'Compressed textual data'
}


class PNGInvalidFormatError(Exception):
    def __init__(self, message):
        super().__init__(message)


class PNGInvalidWriteSequence(Exception):
    def __init__(self, message):
        super().__init__(message)


class PNGChunk:
    def __init__(self, type, file, offset, length):
        self._type = type
        self._f = file
        self._offset = offset
        self._length = length

    def __str__(self):
        return ''

    @property
    def type(self):
        return self._type

    @property
    def type_str(self):
        return self._type.decode('ascii')

    @property
    def is_ancillary(self):
        return (self._type[0] & 0x20) != 0

    @property
    def length(self):
        return self._length

    @property
    def offset(self):
        return self._offset

    def dump(self, f):
        data_length = self._length - 12
        l = [(data_length >> 24) & 0xff, (data_length >> 16) & 0xff, (data_length >> 8) & 0xff, data_length & 0xff]
        assert f.write(bytes(l)) == 4
        assert f.write(self.type) == 4
        f.flush()
        # copy directly from source file to destination
        os.sendfile(f.fileno(), self._f.fileno(), self._offset + 8, self._length - 8)

    @staticmethod
    def create(type, file, offset, length):
        if type == b'IHDR':
            return IHDRChunk(type, file, offset, length)
        elif type == b'PLTE':
            return PLTEChunk(type, file, offset, length)
        elif type == b'tEXt':
            return tEXtChunk(type, file, offset, length)
        return PNGChunk(type, file, offset, length)


formats = {
    0: ('grayscale', (1, 2, 4, 8, 16)),
    2: ('RGB', (8, 16)),
    3: ('palette', (1, 2, 4, 8)),
    4: ('grayscale+alpha', (8, 16)),
    6: ('RGB+alpha', (8, 16))
}


class IHDRChunk(PNGChunk):
    def __init__(self, type, file, offset, length):
        global formats
        assert offset == 8
        assert length == 25
        super().__init__(type, file, offset, length)
        data = os.pread(file.fileno(), 13, offset + 8)
        self._width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]
        self._height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7]
        self._bit_depth = data[8]
        self._color_type = data[9]
        self._compression_method = data[10]
        self._filter_method = data[11]
        self._interlace_method = data[12]
        if self._color_type not in formats:
            raise PNGInvalidFormatError('invalid color type {} in IHDR chunk'.format(self._color_type))
        self._fmt = formats[self._color_type][0]
        if self._bit_depth not in formats[self._color_type][1]:
            raise PNGInvalidFormatError('invalid bit depth {} for color type {} in IHDR chunk'.format(self._bit_depth, self._fmt))

    def __str__(self):
        return '{}x{} pixels, {}, {} bit(s) per sample'.format(self._width, self._height, self._fmt, self._bit_depth)

    @property
    def color_type(self):
        return self._color_type


class PLTEChunk(PNGChunk):
    def __init__(self, type, file, offset, length):
        data_length = length - 12
        if data_length <= 0 or data_length > 768 or data_length % 3 != 0:
            raise PNGInvalidFormatError('invalid length of data in PLTE chunk: {} B'.format(data_length))
        super().__init__(type, file, offset, length)
        self._entries_cnt = data_length // 3

    def __str__(self):
        return '{} entries'.format(self._entries_cnt)


class tEXtChunk(PNGChunk):
    def __init__(self, type, file, offset, length):
        super().__init__(type, file, offset, length)
        if file is not None:
            data = os.pread(file.fileno(), length - 12, offset + 8)
            sep_pos = data.find(b'\x00')
            if sep_pos <= 0:
                raise PNGInvalidFormatError('invalid format of tEXt chunk')
            try:
                self._keyword = data[:sep_pos].decode('iso-8859-1')
                self._text = data[sep_pos+1:].decode('iso-8859-1')
            except UnicodeEncodeError:
                raise PNGInvalidFormatError('invalid character encoding of tEXt chunk')

    def __str__(self):
        return '{}: {}'.format(self._keyword, self._text)

    @staticmethod
    def create(keyword, text):
        raw_keyword = keyword.encode('iso-8859-1')
        raw_text = text.encode('iso-8859-1')
        length = 8 + len(raw_keyword) + 1 + len(raw_text) + 4
        chunk = tEXtChunk(b'tEXt', None, -1, length)
        chunk._keyword = keyword
        chunk._text = text
        return chunk

    def dump(self, f):
        data_length = self._length - 12
        l = [(data_length >> 24) & 0xff, (data_length >> 16) & 0xff, (data_length >> 8) & 0xff, data_length & 0xff]
        assert f.write(bytes(l)) == 4
        assert f.write(self.type) == 4
        checksum = binascii.crc32(self.type)
        raw_keyword = self._keyword.encode('iso-8859-1')
        assert f.write(raw_keyword) == len(raw_keyword)
        checksum = binascii.crc32(raw_keyword, checksum)
        assert f.write(b'\x00') == 1
        checksum = binascii.crc32(b'\x00', checksum)
        raw_text = self._text.encode('iso-8859-1')
        assert f.write(raw_text) == len(raw_text)
        checksum = binascii.crc32(raw_text, checksum)
        c = [(checksum >> 24) & 0xff, (checksum >> 16) & 0xff, (checksum >> 8) & 0xff, checksum & 0xff]
        assert f.write(bytes(c)) == 4


class PNGWriter:
    def __init__(self, path):
        self._path = path
        self._f = None
        self._last_critical_chunk_type = None
        self._last_chunk_type = None
        self._color_type = -1
        self._palette_seen = False

    def __enter__(self):
        dir_path = os.path.dirname(self._path)
        self._f = tempfile.NamedTemporaryFile(mode='w+b', buffering=256, dir=dir_path, delete=False)
        # write header
        assert self._f.write(bytes([137, 80, 78, 71, 13, 10, 26, 10])) == 8
        return self

    def __exit__(self, *args):
        if self._last_critical_chunk_type != b'IEND':
            os.unlink(self._f.name)
            self._f.close()
            raise PNGInvalidWriteSequence('PNG file must end with IEND chunk')
        os.rename(self._f.name, self._path)
        self._f.close()

    def append(self, chunk):
        if self._last_chunk_type is None:
            if not isinstance(chunk, IHDRChunk):
                raise PNGInvalidWriteSequence('PNG file must start with IHDR chunk')
            self._color_type = chunk.color_type
        elif isinstance(chunk, PLTEChunk):
            self._palette_seen = True
        elif chunk.type == b'IDAT':
            if self._color_type == 3 and not self._palette_seen:
                raise PNGInvalidWriteSequence('palette PNG file must have PLTE chunk before first IDAT chunk')
        elif self._last_critical_chunk_type == b'IEND':
            raise PNGInvalidWriteSequence('PNG file cannot have any chunks after IEND chunk')
        self._last_chunk_type = chunk.type
        if not chunk.is_ancillary:
            self._last_critical_chunk_type = chunk.type
        chunk.dump(self._f)


class PNGReader:
    def __init__(self, path):
        self._path = path
        self._f = None

    def __enter__(self):
        self._f = open(self._path, 'rb', buffering=16384)
        # verify header
        header = self._f.read(8)
        if len(header) != 8 or header != bytes([137, 80, 78, 71, 13, 10, 26, 10]):
            raise PNGInvalidFormatError('file has no PNG header')
        # verify IHDR chunk
        raw_IHDR_chunk = self._f.read(25)
        if len(raw_IHDR_chunk) != 25:
            raise PNGInvalidFormatError('file has no IHDR chunk')
        raw_IHDR_chunk_data_length = (raw_IHDR_chunk[0] << 24) | (raw_IHDR_chunk[1] << 16) | (raw_IHDR_chunk[2] << 8) | raw_IHDR_chunk[3]
        raw_IHDR_chunk_type = raw_IHDR_chunk[4:8]
        if raw_IHDR_chunk_data_length != 13 or raw_IHDR_chunk_type != b'IHDR':
            raise PNGInvalidFormatError('file has invalid IHDR chunk')
        return self

    def __exit__(self, *args):
        self._f.close()
        self._f = None

    def chunks(self):
        # start just after PNG header
        offset = 8
        assert self._f.seek(offset) == offset
        while True:
            header = self._f.read(8)
            if len(header) != 8:
                raise PNGInvalidFormatError('file is truncated')
            data_length = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3]
            type = header[4:8]
            yield PNGChunk.create(type, self._f, offset, 8 + data_length + 4)
            # skip chunk data and CRC32 checksum
            offset += 8 + data_length + 4
            if self._f.seek(offset) != offset:
                raise PNGInvalidFormatError('file is truncated')
            if type == b'IEND':
                # this should be the last chunk
                break
        # make sure that there is nothing more after last chunk
        if self._f.read() != bytes():
            file_size = os.path.getsize(self._path)
            raise PNGInvalidFormatError('file has {} bytes of trash after PNG IHDR chunk'.format(file_size - offset))


def describe_PNG_chunk(chunk, args, is_needed=True):
    global chunk_types
    if not args.verbose:
        return
    mark = ' ' if is_needed else '-'
    t = 'ancillary' if chunk.is_ancillary else ' critical'
    d = chunk_types[chunk.type] if chunk.type in chunk_types else ''
    print('  {} {}  {}  {:6d} B @ {:6d}  {}'.format(mark, chunk.type_str, t, chunk.length, chunk.offset, d))
    s = str(chunk)
    if s != '':
        print('        ' + s)


def is_PNG_chunk_needed(chunk, color_type):
    if chunk.type == b'PLTE' and color_type != 3:
        # palette chunk in non-palette file is only informative
        return False
    if chunk.type == b'tRNS' and color_type not in (4, 6):
        # transparency information may be useful in files without alpha channel
        return True
    return not chunk.is_ancillary


def find_PNG_files(dir_path, file_paths):
    for name in os.listdir(dir_path):
        path = dir_path + '/' + name if dir_path != '.' else name
        if os.path.isdir(path):
            find_PNG_files(path, file_paths)
        elif os.path.isfile(path) and name.endswith('.png'):
            file_paths.append(path)


def main():
    parser = argparse.ArgumentParser(description='Strip optional chunks of PNG files.')
    parser.add_argument('-v', '--verbose', action='store_true', default=False,
                        help='output information about all chunks')
    parser.add_argument('-n', '--dry-run', action='store_true', default=False,
                        help="don't modify file, useful in verbose mode")
    parser.add_argument('--add-copyright', metavar='TEXT', dest='copyright', default=None,
                        help='add copyright TEXT to PNG files')
    parser.add_argument('paths', metavar='PATH', nargs='+',
                        help='PNG file or directory to search for PNG files')
    args = parser.parse_args()

    if args.copyright is not None:
        if args.dry_run:
            print("clean_PNG_files.py: options --add-copyright and --dry-run can't be used together", file=sys.stderr)
            return 1
        try:
            args.copyright.encode('iso-8859-1')
        except UnicodeEncodeError:
            print('clean_PNG_files.py: copyright text can only contain ISO 8859-1 characters', file=sys.stderr)
            return 1

    # collect list of file paths
    have_directories = False
    file_paths = list()
    for path in args.paths:
        if os.path.isdir(path):
            have_directories = True
            find_PNG_files(path, file_paths)
        elif os.path.isfile(path):
            file_paths.append(path)
        else:
            print('clean_PNG_files.py: invalid argument {}: no such file or directory'.format(path), file=sys.stderr)
            return 1

    if args.verbose and have_directories:
        print('found {} files'.format(len(file_paths)), flush=True)

    status = 0
    for path in file_paths:
        if args.verbose:
            print('file ' + path)
        try:
            if args.dry_run:
                # iterate over chunks to check if file is valid
                # and, optionally, print information about them
                with PNGReader(path) as input_f:
                    color_type = -1
                    for chunk in input_f.chunks():
                        if chunk.type == b'IHDR':
                            color_type = chunk.color_type
                        is_needed = is_PNG_chunk_needed(chunk, color_type)
                        describe_PNG_chunk(chunk, args, is_needed)
            else:
                with PNGWriter(path) as output_f:
                    with PNGReader(path) as input_f:
                        color_type = -1
                        for chunk in input_f.chunks():
                            if chunk.type == b'IHDR':
                                color_type = chunk.color_type
                            is_needed = is_PNG_chunk_needed(chunk, color_type)
                            describe_PNG_chunk(chunk, args, is_needed)
                            if chunk.type == b'IEND' and args.copyright is not None:
                                copyright = tEXtChunk.create('Copyright', args.copyright)
                                output_f.append(copyright)
                            if is_needed:
                                output_f.append(chunk)
        except PNGInvalidFormatError as ex:
            sys.stdout.flush()
            print('file {} is broken: {}'.format(path, str(ex)), file=sys.stderr)
            status = 1 
        except PNGInvalidWriteSequence as ex:
            sys.stdout.flush()
            print('error writing file {}: {}'.format(path, str(ex)), file=sys.stderr)
            status = 1
        except Exception as ex:
            sys.stdout.flush()
            print('error processing file {}: {}'.format(path, str(ex)), file=sys.stderr)
            status = 1

    return status


if __name__ == '__main__':
    sys.exit(main())
