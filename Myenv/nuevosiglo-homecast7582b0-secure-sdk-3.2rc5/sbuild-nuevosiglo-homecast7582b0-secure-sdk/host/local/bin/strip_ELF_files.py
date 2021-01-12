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
import os.path
import stat
import shutil
from collections import namedtuple, OrderedDict
import struct
import binascii
import tempfile
import argparse
import subprocess
import traceback
from multiprocessing import Pool


class QBRandomAccessStream:
    def __init__(self, f, path, is_little_endian):
        self._file = f
        self._length = os.path.getsize(path)
        self._path = path
        self._is_little_endian = is_little_endian
        self._buffer = None
        self._buffer_offset = 0

    @property
    def file(self):
        return self._file

    @property
    def length(self):
        return self._length

    @property
    def is_little_endian(self):
        return self._is_little_endian

    def copy(self):
        return QBRandomAccessStream(self._file, self._path, self._is_little_endian)

    def read(self, offset, n=None):
        if n is None:
            n = self._length - offset
        if not self._is_in_buffer(offset, n):
            self._fill_buffer(offset, n)
        offset -= self._buffer_offset
        return self._buffer[offset:offset+n]

    def read_16(self, offset):
        if not self._is_in_buffer(offset, 2):
            self._fill_buffer(offset, 2)
        offset -= self._buffer_offset
        if self._is_little_endian:
            return (self._buffer[offset+1] << 8) | self._buffer[offset];
        else:
            return (self._buffer[offset] << 8) | self._buffer[offset+1];

    def read_32(self, offset):
        if not self._is_in_buffer(offset, 4):
            self._fill_buffer(offset, 4)
        offset -= self._buffer_offset
        data = self._buffer[offset:offset+4]
        if self._is_little_endian:
            return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
        else:
            return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

    def read_64(self, offset):
        if self._is_little_endian:
            return self.read_32(offset) | (self.read_32(offset+4) << 32)
        else:
            return (self.read_32(offset) << 32) | self.read_32(offset+4)

    def write_32(self, offset, v):
        if not self._is_in_buffer(offset, 4):
            self._fill_buffer(offset, 4)
        offset -= self._buffer_offset
        if not isinstance(self._buffer, bytearray):
            self._buffer = bytearray(self._buffer)
        if self._is_little_endian:
            self._buffer[offset] = v & 0xff
            self._buffer[offset+1] = (v >> 8) & 0xff
            self._buffer[offset+2] = (v >> 16) & 0xff
            self._buffer[offset+3] = (v >> 24) & 0xff
        else:
            self._buffer[offset] = (v >> 24) & 0xff
            self._buffer[offset+1] = (v >> 16) & 0xff
            self._buffer[offset+2] = (v >> 8) & 0xff
            self._buffer[offset+3] = v & 0xff
        self._file.seek(self._buffer_offset + offset)
        self._file.write(self._buffer[offset:offset+4])
        self._file.flush()

    def _is_in_buffer(self, offset, length):
        buffer_length = len(self._buffer) if self._buffer is not None else 0
        return offset + length <= self._buffer_offset + buffer_length and offset >= self._buffer_offset

    def _fill_buffer(self, offset, length):
        length = length if length >= 4096 else 4096
        try:
            self._file.seek(offset)
            self._buffer = self._file.read(length)
            self._buffer_offset = offset
        except:
            raise RuntimeError('error reading {}B @ {:x} from {}'.format(length, offset, self._path))


ELFFileHeader = namedtuple('ELFFileHeader',
                           ['ident', 'type', 'machine', 'version', 'entry',
                            'program_header_table_offset',
                            'section_header_table_offset',
                            'flags', 'header_size',
                            'program_header_table_entry_size',
                            'program_header_table_entries_count',
                            'section_header_table_entry_size',
                            'section_header_table_entries_count',
                            'section_header_string_table_index'])


class QBELFSegment:
    def __init__(self, stream, header_offset, is_64_bit):
        self._stream = stream
        self._header_offset = header_offset
        self._is_little_endian = stream.is_little_endian
        self._is_64_bit = is_64_bit
        if is_64_bit:
            self._type = stream.read_32(header_offset)
            self._flags = stream.read_32(header_offset+4)
            self._file_offset = stream.read_64(header_offset+8)
            self._file_size = stream.read_64(header_offset+32)
        else:
            self._type = stream.read_32(header_offset)
            self._file_offset = stream.read_32(header_offset+4)
            self._file_size = stream.read_32(header_offset+16)
            self._flags = stream.read_32(header_offset+24)

    @property
    def file_offset(self):
        return self._file_offset

    @property
    def file_size(self):
        return self._file_size


class QBELFSection:
    def __init__(self, stream, header_offset, is_64_bit):
        self._stream = stream
        self._header_offset = header_offset
        self._is_little_endian = stream.is_little_endian
        self._is_64_bit = is_64_bit
        if is_64_bit:
            self._type = stream.read_32(header_offset+4)
            self._flags = stream.read_64(header_offset+8)
            self._offset = stream.read_64(header_offset+24)
            self._length = stream.read_64(header_offset+32)
        else:
            self._type = stream.read_32(header_offset+4)
            self._flags = stream.read_32(header_offset+4)
            self._offset = stream.read_32(header_offset+16)
            self._length = stream.read_32(header_offset+20)

    @property
    def type(self):
        return self._type

    @property
    def data_offset(self):
        return self._offset

    @property
    def data_length(self):
        return self._length

    def get_data(self):
        return self._stream.read(self._offset, self._length)


class QBELFStringTableSection(QBELFSection):
    def __init__(self, stream, header_offset, is_64_bit):
        super().__init__(stream, header_offset, is_64_bit)
        self._stream = stream.copy()

    def get_string(self, index):
        assert index < self._length
        if index == 0:
            return None
        data = self._stream.read(self._offset + index, 256)
        null_pos = data.find(0)
        if null_pos < 0:
            data = self._stream.read(self._offset + index)
            null_pos = data.find(0)
            assert null_pos > 0
        return data[:null_pos].decode('utf-8')


class QBELFFile:
    def __init__(self, path, prefix, sbuild, stream):
        self._path = path
        self._paths = set()
        self._paths.add(path)
        self._prefix = prefix
        self._sbuild = sbuild
        raw_header = stream.read(0, 18)
        assert raw_header[4] in (1, 2)
        self._is_64_bit = raw_header[4] == 2
        assert raw_header[5] in (1, 2)
        self._is_little_endian = raw_header[5] == 1
        self._have_symbol_table = False
        self._build_id = None
        self._file_type = stream.read_16(16)
        self._min_size = 0
        if self._file_type == 2 or self._file_type == 3:
            # this is an executable or a shared library
            self._parse(stream)

    @property
    def path(self):
        return self._path

    @property
    def paths(self):
        return sorted(self._paths)

    @property
    def logical_path(self):
        return os.path.join(self._prefix, os.path.basename(self._path))

    @property
    def sbuild_relative_path(self):
        return os.path.relpath(self._path, self._sbuild.root_path)

    @property
    def build_id(self):
        return self._build_id

    def has_symbol_table(self):
        return self._have_symbol_table

    def set_build_id(self, build_id):
        assert self._build_id is None
        self._build_id = binascii.hexlify(build_id).decode('ascii')

    def _find_cross_path(self):
        prefix = self._sbuild.cross_path + '/'
        for path in self._paths:
            if path.startswith(prefix):
                return path
        return None

    def _parse(self, stream):
        header = self._read_header(stream)
        # calculate minimal file size containing program header table and section header table
        program_header_table_size = header.program_header_table_entry_size * header.program_header_table_entries_count
        if program_header_table_size > 0:
            self._min_size = max(self._min_size, header.program_header_table_offset + program_header_table_size)
        section_header_table_size = header.section_header_table_entry_size * header.section_header_table_entries_count
        if section_header_table_size > 0:
            self._min_size = max(self._min_size, header.section_header_table_offset + section_header_table_size)
        # iterate over all segments to update minimal file size
        for i in range(header.program_header_table_entries_count):
            header_offset = header.program_header_table_offset + i * header.program_header_table_entry_size
            segment = QBELFSegment(stream, header_offset, self._is_64_bit)
            if segment.file_size > 0:
                self._min_size = max(self._min_size, segment.file_offset + segment.file_size)
        # prepare section header string table, needed to get section names
        offset = header.section_header_table_offset + header.section_header_string_table_index * header.section_header_table_entry_size
        string_table_section = QBELFStringTableSection(stream, offset, self._is_64_bit)
        # iterate over all sections
        for i in range(header.section_header_table_entries_count):
            header_offset = header.section_header_table_offset + i * header.section_header_table_entry_size
            section_name_index = stream.read_32(header_offset)
            section_name = string_table_section.get_string(section_name_index)
            section = QBELFSection(stream, header_offset, self._is_64_bit)
            if section.type == 7 and section_name == '.note.gnu.build-id':
                # section type SHT_NOTE
                self._parse_GNU_build_id(section)
            elif section.type == 2:
                # section type SHT_SYMTAB
                assert not self._have_symbol_table
                self._have_symbol_table = True
            # update minimal file size if section is not SHT_NOBITS
            if section.data_length > 0 and section.type != 8:
                self._min_size = max(self._min_size, section.data_offset + section.data_length)
        # confirm that file is not too short
        assert stream.length >= self._min_size

    def _parse_GNU_build_id(self, section):
        if section.data_length < 18:
            return
        section_data = section.get_data()
        # format of this section:
        #   4 bytes: length of name, including NULL byte
        #   4 bytes: length of data
        #   4 bytes: type of note (we expect NT_GNU_BUILD_ID == 3)
        #   N bytes: NULL-terminated name (we expect 'GNU')
        #   padding to ensure 4 byte alignment of data
        #   X bytes: hash data
        fmt = '<LLL4s' if self._is_little_endian else '>LLL4s'
        fields = struct.unpack(fmt, section_data[:16])
        if fields[0] != 4 or fields[1] < 2 or fields[2] != 3:
            # unsupported note format
            return
        assert 12 + fields[0] + fields[1] == len(section_data)
        assert fields[3] == b'GNU\0'
        self._build_id = binascii.hexlify(section_data[16:]).decode('ascii')

    def _fix_build_id_section_type(self, path):
        with open(path, 'r+b') as f:
            stream = QBRandomAccessStream(f, path, self._is_little_endian)
            header = self._read_header(stream)
            # start with section header string table, needed to get section names
            offset = header.section_header_table_offset + header.section_header_string_table_index * header.section_header_table_entry_size
            string_table_section = QBELFStringTableSection(stream, offset, self._is_64_bit)
            # iterate over all sections
            for i in range(header.section_header_table_entries_count):
                header_offset = header.section_header_table_offset + i * header.section_header_table_entry_size
                section_name_index = stream.read_32(header_offset)
                section_name = string_table_section.get_string(section_name_index)
                section_type = stream.read_32(header_offset + 4)
                if section_name == '.note.gnu.build-id' and section_type != 7:
                    # change section type to SHT_NOTE == 7
                    stream.write_32(header_offset + 4, 7)
                    break

    def _read_header(self, stream):
        if self._is_64_bit:
            fmt = '<16sHHLQQQLHHHHHH' if self._is_little_endian else '>16sHHLQQQLHHHHHH'
            raw_header = stream.read(0, 64)
        else:
            fmt = '<16sHHLLLLLHHHHHH' if self._is_little_endian else '>16sHHLLLLLHHHHHH'
            raw_header = stream.read(0, 52)
        return ELFFileHeader(*struct.unpack(fmt, raw_header))

    @classmethod
    def try_load(cls, path, prefix, sbuild):
        with open(path, 'r+b') as f:
            header = f.read(18)
            if len(header) != 18 or header[:4] != b'\x7fELF':
                # doesn't look like an ELF file
                return None
            is_little_endian = header[5] == 1
            stream = QBRandomAccessStream(f, path, header[5] == 1)
            return cls(path, prefix, sbuild, stream)

    def add_path(self, path):
        self._paths.add(path)

    def insert_build_id(self, build_id):
        assert self._build_id is None
        # generate .note.gnu.build-id section
        fmt = ('<' if self._is_little_endian else '>') + 'LLL4s{}s'.format(len(build_id))
        section = struct.pack(fmt, 4, len(build_id), 3, b'GNU\0', build_id)
        # insert new section using objcopy utility
        with tempfile.NamedTemporaryFile(mode='wb', dir=self._sbuild.tmp_path) as section_file:
            section_file.write(section)
            section_file.flush()
            dest_dir_path = os.path.dirname(self._path)
            with tempfile.NamedTemporaryFile(mode='w+b', dir=dest_dir_path) as output_file:
                output_file.close()
                self._sbuild.call('objcopy',
                                  '--add-section',
                                  '.note.gnu.build-id={}'.format(section_file.name),
                                  '--set-section-flags',
                                  '.note.gnu.build-id=contents,readonly',
                                  self._path, output_file.name,
                                  ignore_stderr=True)
                # make sure the file was updated correctly and replace with new file
                assert self._min_size > 0 and os.path.getsize(output_file.name) > self._min_size
                self._fix_build_id_section_type(output_file.name)
                shutil.copystat(self._path, output_file.name)
                os.rename(output_file.name, self._path)
        # fix remaining hard links if needed 
        if len(self._paths) > 1:
            for other_path in self._paths:
                if other_path != self._path:
                    os.remove(other_path)
                    os.link(self._path, other_path)
        self.set_build_id(build_id)

    def strip(self):
        assert self._have_symbol_table
        assert self._build_id is not None
        # create separate debugging information file
        debug_path = os.path.join(self._sbuild.debug_path, self._prefix)
        os.makedirs(debug_path, mode=0o755, exist_ok=True)
        debug_info_file_path = os.path.join(debug_path, os.path.basename(self._path) + '.debug')
        if os.path.exists(debug_info_file_path):
            os.remove(debug_info_file_path)
        self._sbuild.call('objcopy', '--only-keep-debug', self._path, debug_info_file_path)
        shutil.copystat(self._path, debug_info_file_path)
        # create symbolic link with a name corresponding to build ID, pointing to a debugging information file
        build_id_path = os.path.join(self._sbuild.debug_path, '.build-id', self._build_id[:2])
        os.makedirs(build_id_path, mode=0o755, exist_ok=True)
        link_path = os.path.join(build_id_path, self._build_id[2:] + '.debug')
        if os.path.exists(link_path):
            os.remove(link_path)
        os.symlink(os.path.join('..', '..', self._prefix, os.path.basename(self._path) + '.debug'), link_path)
        # remove debugging information from original file
        self._sbuild.call('strip', self._path)
        shutil.copystat(debug_info_file_path, self._path)
        cross_path = self._find_cross_path()
        if cross_path is not None:
            # insert .gnu.debuglink section into the stripped file
            self._sbuild.call('objcopy', '--add-gnu-debuglink=' + debug_info_file_path, cross_path)
            # create a hard link to the debugging information file in .debug subdirectory, needed by valgrind
            local_debug_dir_path = os.path.join(os.path.dirname(cross_path), '.debug')
            os.makedirs(local_debug_dir_path, mode=0o755, exist_ok=True)
            local_debug_info_file_path = os.path.join(local_debug_dir_path, os.path.basename(self._path) + '.debug')
            if os.path.exists(local_debug_info_file_path):
                os.remove(local_debug_info_file_path)
            os.link(debug_info_file_path, local_debug_info_file_path)
        # fix hard links if needed 
        if len(self._paths) > 1:
            for other_path in self._paths:
                if other_path != self._path:
                    os.remove(other_path)
                    os.link(self._path, other_path)


class QBSBuild:
    def __init__(self, root_path, cross_arch, target_only):
        self._root_path = os.path.abspath(root_path)
        self._cross_arch = cross_arch
        self._target_path = os.path.join(root_path, 'target', 'root')
        self._cross_path = os.path.join(root_path, 'host', 'cross', cross_arch)
        if not os.path.isdir(self._target_path):
            raise RuntimeError('directory {} doesn\'t exist'.format(self._target_path))
        if not os.path.isdir(self._cross_path):
            raise RuntimeError('directory {} doesn\'t exist'.format(self._cross_path))
        self._search_paths = OrderedDict()
        self._register_default_paths(target_only)

    @property
    def root_path(self):
        return self._root_path

    @property
    def cross_path(self):
        return self._cross_path

    @property
    def tmp_path(self):
        return os.path.join(self._root_path, '_tmp')

    @property
    def debug_path(self):
        return os.path.join(self._root_path, 'debug')

    def _register_default_paths(self, target_only=False):
        dir_paths = [ 'lib', 'sbin', 'usr/bin', 'usr/lib', 'usr/sbin',
                      'usr/local/bin', 'usr/local/lib', 'usr/local/sbin' ]
        for dir_path in ['bin'] + dir_paths:
            abs_path = os.path.normpath(os.path.join(self._target_path, dir_path))
            if os.path.isdir(abs_path):
                self._search_paths[os.path.realpath(abs_path)] = dir_path
        if target_only:
            return
        for dir_path in dir_paths:
            abs_path = os.path.normpath(os.path.join(self._cross_path, dir_path))
            if os.path.isdir(abs_path):
                self._search_paths[os.path.realpath(abs_path)] = dir_path

    def add_search_path(self, dir_path):
        abs_path = os.path.normpath(os.path.join(self._root_path, dir_path))
        if not os.path.isdir(abs_path):
            raise RuntimeError('directory {} doesn\'t exist in {}'.format(dir_path, self._root_path))
        self._search_paths[os.path.realpath(abs_path)] = dir_path

    def find_ELF_files(self):
        """Generate the list of all ELF files in all registered directories."""
        files = dict()
        for search_path in self._search_paths.keys():
            for dir_path, dir_names, file_names in os.walk(search_path):
                for file_name in file_names:
                    if file_name.endswith('.debug'):
                        continue
                    file_path = os.path.join(dir_path, file_name)
                    st = os.stat(file_path, follow_symlinks=False)
                    if not stat.S_ISREG(st.st_mode):
                        continue
                    if (st.st_dev, st.st_ino) not in files:
                        relative_dir_path = os.path.dirname(os.path.relpath(file_path, search_path))
                        prefix = os.path.join(self._search_paths[search_path], relative_dir_path)
                        if (st.st_mode & stat.S_IWUSR) == 0:
                            # file is not writable by the owner, we have to fix it
                            access_mode_mask = stat.S_ISUID | stat.S_ISGID | stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO
                            os.chmod(file_path, (st.st_mode & access_mode_mask) | stat.S_IWUSR)
                        elf_file = QBELFFile.try_load(file_path, prefix, self)
                        if elf_file is not None:
                            files[(st.st_dev, st.st_ino)] = elf_file
                    else:
                        files[(st.st_dev, st.st_ino)].add_path(file_path)
        return list(files.values())

    def call(self, *args, ignore_stderr=False):
        exe_name = self._cross_arch + '-' + args[0]
        exe_path = os.path.join(self._root_path, 'host', 'cross', 'bin', exe_name)
        if not os.path.isfile(exe_path):
            raise RuntimeError('can\'t find {} executable'.format(exe_name))
        err = subprocess.DEVNULL if ignore_stderr else None
        proc = subprocess.Popen([exe_name] + list(args)[1:], executable=exe_path, shell=False,
                                stdout=subprocess.PIPE, stderr=err)
        out, _ = proc.communicate()
        if proc.returncode != 0:
            raise RuntimeError(' '.join(args) + ' failed with code {}'.format(proc.returncode))
        return out


def insert_build_id(elf_file):
    try:
        with open('/dev/urandom', 'rb') as f:
            build_id = f.read(20)
        elf_file.insert_build_id(build_id)
        return build_id
    except Exception as ex:
        print('strip_ELF_files: ' + str(ex), file=sys.stderr)
        traceback.print_exc()
        return None


def strip_ELF_file(elf_file):
    try:
        elf_file.strip()
        return True
    except Exception as ex:
        print('strip_ELF_files: ' + str(ex), file=sys.stderr)
        traceback.print_exc()
        return False


def main():
    parser = argparse.ArgumentParser(description='Strip ELF executables and shared libraries.')
    parser.add_argument('--target-only', action='store_true', default=False,
                        help="strip only files in $SRM_ROOT/target/root")
    parser.add_argument('extra_directories', metavar='DIR', nargs='*',
                        help='extra directories to search for ELF files')
    args = parser.parse_args()

    if 'SRM_ROOT' not in os.environ:
        print('strip_ELF_files: SRM_ROOT variable is not set', file=sys.stderr)
        return 1
    if 'CROSS_ARCH' not in os.environ:
        print('strip_ELF_files: CROSS_ARCH variable is not set', file=sys.stderr)
        return 1

    try:
        sbuild = QBSBuild(os.environ['SRM_ROOT'], os.environ['CROSS_ARCH'], args.target_only)
        # add custom directories specified on command line
        for extra_dir in args.extra_directories:
            sbuild.add_search_path(extra_dir)
        all_ELF_files = sbuild.find_ELF_files()
    except Exception as ex:
        print('strip_ELF_files: ' + str(ex), file=sys.stderr)
        traceback.print_exc()
        return 1

    files_to_be_stripped = list(filter(lambda f: f.has_symbol_table(), all_ELF_files))

    # detect path conflicts, i.e. same file copied into $SRM_ROOT/target/root
    # and $SRM_ROOT/host/cross/$CROSS_ARCH: they should be hard links!
    files_by_logical_path = dict()
    for elf_file in files_to_be_stripped:
        logical_path = elf_file.logical_path
        if logical_path in files_by_logical_path:
            other_file = files_by_logical_path[logical_path]
            print('strip_ELF_files: conflicting files {} and {}'.format(other_file.sbuild_relative_path, elf_file.sbuild_relative_path), file=sys.stderr)
        else:
            files_by_logical_path[logical_path] = elf_file
    if len(files_by_logical_path) != len(files_to_be_stripped):
        return 1

    files_without_build_id = list(filter(lambda f: f.build_id is None, files_to_be_stripped))

    all_OK = True
    with Pool() as pool:
        if len(files_without_build_id) > 0:
            results = pool.map(insert_build_id, files_without_build_id)
            for status, f in zip(results, files_without_build_id):
                if status is not None:
                    f.set_build_id(status)
                else:
                    all_OK = False
        if all_OK and len(files_to_be_stripped) > 0:
            for status in pool.map(strip_ELF_file, files_to_be_stripped, 2):
                all_OK = all_OK and status

    return 0 if all_OK else 1


if __name__ == '__main__':
    sys.exit(main())
