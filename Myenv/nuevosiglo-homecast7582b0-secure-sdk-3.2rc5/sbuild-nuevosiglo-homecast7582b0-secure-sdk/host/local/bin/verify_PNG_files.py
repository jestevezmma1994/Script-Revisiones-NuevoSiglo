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


class PNGInvalidFormatError(Exception):
    def __init__(self, message):
        super().__init__(message)


class PNGChunk:
    def __init__(self, type):
        self._type = type

    @property
    def type(self):
        return self._type

    @property
    def is_ancillary(self):
        return (self._type[0] & 0x20) != 0


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
            raise PNGInvalidFormatError('file has no PNG IHDR chunk')
        raw_IHDR_chunk_data_length = (raw_IHDR_chunk[0] << 24) | (raw_IHDR_chunk[1] << 16) | (raw_IHDR_chunk[2] << 8) | raw_IHDR_chunk[3]
        raw_IHDR_chunk_type = raw_IHDR_chunk[4:8]
        if raw_IHDR_chunk_data_length != 13 or raw_IHDR_chunk_type != b'IHDR':
            raise PNGInvalidFormatError('file has invalid PNG IHDR chunk')
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
            yield PNGChunk(type)
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


def check_PNG_file(path):
    tEXt_chunk_seen = False
    try:
        with PNGReader(path) as f:
            for chunk in f.chunks():
                if chunk.type == b'tEXt' and not tEXt_chunk_seen:
                    # accept single instance of tEXt chunk, clean_PNG.py script puts copyright info there
                    tEXt_chunk_seen = True
                elif chunk.type == b'tRNS':
                    # tRNS chunk is ancillary, but sometimes contains valuable information
                    pass
                elif chunk.is_ancillary:
                    t = chunk.type.decode('ascii')
                    print('file {} contains unneeded ancillary chunk {}'.format(path, t), file=sys.stderr)
                    return False
    except PNGInvalidFormatError as ex:
        print('file {} is broken: {}'.format(path, str(ex)), file=sys.stderr)
        return False
    except:
        print('error reading file {}: {}'.format(path, str(ex)), file=sys.stderr)
        return False
    return True


def find_PNG_files(dir_path, file_paths):
    for name in os.listdir(dir_path):
        path = dir_path + '/' + name
        if os.path.isdir(path):
            find_PNG_files(path, file_paths)
        elif os.path.isfile(path) and name.endswith('.png'):
            file_paths.append(path)


def main():
    # collect list of file paths
    file_paths = list()
    for path in sys.argv[1:]:
        if os.path.isdir(path):
            find_PNG_files(path, file_paths)
        elif os.path.isfile(path):
            file_paths.append(path)
        else:
            print('invalid argument {}: no such file or directory'.format(path), file=sys.stderr)
            return 1

    all_OK = True
    if len(file_paths) > 30:
        # multiprocess version
        from multiprocessing import Pool
        with Pool(3) as pool:
            for file_OK in pool.map(check_PNG_file, file_paths, 8):
                all_OK = all_OK and file_OK
    else:
        # simple versions
        for path in file_paths:
            file_OK = check_PNG_file(path)
            all_OK = all_OK and file_OK

    return 0 if all_OK else 1


if __name__ == '__main__':
    sys.exit(main())
