#############################################################################
# TiVo Poland Sp. z o.o. Software License Version 1.0
#
# Copyright (C) 2019-2020 TiVo Poland Sp. z o.o. All rights reserved.
#
# Any rights which are not expressly granted in this License are entirely and
# exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
# modify, translate, reverse engineer, decompile, disassemble, or create
# derivative works based on this Software. You may not make access to this
# Software available to others in connection with a service bureau,
# application service provider, or similar business, or make any other use of
# this Software without express written permission from TiVo Poland Sp. z o.o.
#
# Any User wishing to make use of this Software must contact
# TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
# includes, but is not limited to:
# (1) integrating or incorporating all or part of the code into a product for
#     sale or license by, or on behalf of, User to third parties;
# (2) distribution of the binary or source code to third parties for use with
#     a commercial product sold or licensed by, or on behalf of, User.
#############################################################################

"""Abstractions of several parts of build system to be validated."""
import datetime
import json
import os
import re
import sre_constants
import stat
from hashlib import md5
from typing import Any, Callable, Dict, Generator, IO, List, Optional, Tuple, Union
from xml.etree import ElementTree

from M2Crypto import BIO, X509

from cerberus.global_status import GlobalStatus


class ConfigAbstract:
    """Abstract class representing validators for several parts of configuration: from files to dictionaries."""

    def __init__(self, sbuild_path: str, _: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        self.entries = {}  # type: Dict[str, Any]
        self._sbuild_path = sbuild_path
        self._name = name
        self._gstatus = status

    def _open(self, filename: str, mode: str) -> Union[IO[bytes], IO[str]]:
        real_filename = os.path.realpath(filename)
        real_tree_path = os.path.realpath(self._sbuild_path)

        if real_filename.startswith(real_tree_path):
            return open(filename, mode)
        # TO DO: try to read in the file it points to withing the root path
        # TO DO: root path is unknown, we only know sbuild_path which is not the same thing
        raise IOError

    def has_entry(self, entry: str) -> bool:
        """Check that non-empty entry is present among entries."""
        return entry in self.entries and self.entries[entry] is not None

    def has_entry_value(self, entry: str, regex: str) -> bool:
        """Check that entry is present and is a string."""
        if entry in self.entries:
            if isinstance(self.entries[entry], str):
                return bool(re.match(regex, self.entries[entry]))
            raise TypeError
        return False

    def assert_entry(self, test_case: str, entry: str, strict: bool = True) -> None:
        """Assert that expected entry is present in entries."""
        if entry not in self.entries:
            if strict:
                self.err(test_case, "Variable [%s] does not exist!" % entry)
            else:
                self.wrn(test_case, "Variable [%s] does not exist, but strict check is disabled" % entry)
        else:
            self.ok(test_case, "Variable [%s] -> [%s]" % (entry, self.entries[entry]))

    def assert_entry_value(self, test_case: str, entry: str, regex: str, strict: bool = True) -> None:
        """Assert that entry has a correct value."""
        if entry not in self.entries:
            if strict:
                self.err(test_case, "Variable [%s] does not exist! Expected [%s]" % (entry, regex))
            else:
                self.wrn(test_case, "Variable [%s] does not exist, Expected [%s],"
                                    " but strict check is disabled" % (entry, regex))
        else:
            value = self.entries[entry]

            if re.match(regex, value):
                self.ok(test_case, "Variable [%s] -> [%s] checks out against [%s]" % (entry, value, regex))
            else:
                if strict:
                    self.err(test_case, "Illegal variable value [%s] -> [%s] :"
                                        " does not match pattern [%s]" % (entry, value, regex))
                else:
                    self.wrn(test_case, "Illegal variable value [%s] -> [%s] : does not match pattern [%s],"
                                        " but strict check is disabled" % (entry, value, regex))

    def _append_entry(self, entry: str, value: Optional[str]) -> None:
        self.entries[entry] = value

    def wrn(self, test_case: str, msg: str) -> None:
        """Add test case with warning status."""
        self._gstatus.update_status(test_case, self._name, "WRN", msg)

    def err(self, test_case: str, msg: str) -> None:
        """Add test case with error status."""
        self._gstatus.update_status(test_case, self._name, "ERR", msg)

    def ok(self, test_case: str, msg: str) -> None:  # pylint: disable=invalid-name
        """Add test case with OK status."""
        self._gstatus.update_status(test_case, self._name, "OK", msg)

    def print_tc_stat(self, test_case: str) -> None:
        """Print status for a test case."""
        self._gstatus.print_tc_status(test_case, self._name)

    def num_status(self, status: str) -> int:
        """Get numeric value of status."""
        return self._gstatus.num_status(self._name, status)

    def print_stats(self, stat_type: str = ".*") -> None:
        """Print states from global status."""
        self._gstatus.print_stats(self._name, stat_type)


class FileSystem(ConfigAbstract):
    """Validator for files in target system."""

    def __init__(self, sbuild_path: str, path: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(FileSystem, self).__init__(sbuild_path, path, name, status)

        for filename, fstat, target, md5sum in self.find_files(os.path.join(sbuild_path, path)):
            fstat_dict = self._fstat_to_dict(fstat)
            if target is not None:
                fstat_dict["target"] = target
            if md5sum is not None:
                fstat_dict["md5sum"] = md5sum

            self.entries[filename] = fstat_dict

    def __str__(self) -> str:
        """Stringify as list of space-separated pairs of file entries."""
        return '\n'.join("%s %s" % (entry, self.entries[entry]["st_ino"]) for entry in self.entries)

    def find_files(self, directory: str) \
            -> Generator[Tuple[str, os.stat_result, Optional[str], Optional[str]], None, None]:
        """Find files in a directory and yield extended metadata about all files."""
        for root, dirs, files in os.walk(directory):
            for basename in files + dirs:
                filename = os.path.join(root, basename)
                stats = os.lstat(filename)
                target = None
                md5sum = None

                if stat.S_ISLNK(stats.st_mode):
                    target = os.readlink(filename)
                else:
                    if not any([f(stats.st_mode)
                                for f in [stat.S_ISCHR, stat.S_ISDIR, stat.S_ISLNK, stat.S_ISBLK, stat.S_ISFIFO]]):
                        md5_input = self._open(filename, 'rb').read()
                        assert isinstance(md5_input, bytes)
                        md5sum = md5(md5_input).hexdigest()
                yield filename, stats, target, md5sum

    @staticmethod
    def _fstat_to_dict(fstat: Any) -> Dict[str, Any]:
        return {key: getattr(fstat, key) for key in {"st_mode", "st_ino", "st_dev", "st_nlink", "st_uid",
                                                     "st_gid", "st_size", "st_atime", "st_mtime", "st_ctime"}}

    def lookup_entries(self, filename: str) -> Optional[str]:
        """Find an entry for the given name of file."""
        # Look for entire path
        if re.match(".*/.*", filename):
            full_path = os.path.relpath(os.path.realpath(os.path.join(self._sbuild_path, filename)), os.getcwd())
            if full_path in self.entries:
                return full_path
        else:
            # Look for matching leaves
            pattern = '{}{}$'.format(os.sep, filename)
            regex = re.compile(pattern)
            for entry in self.entries:
                if regex.search(entry):
                    return entry

        return None

    def has_file(self, filename: str) -> bool:
        """Check that there is such a file among entries."""
        return bool(self.lookup_entries(filename))

    def nassert_file(self, test_case: str, filename: str, strict: bool = True) -> None:
        """Assert that the file doesn't exist."""
        full_path = self.lookup_entries(filename)

        if not full_path:
            self.ok(test_case, "As expected file not found [%s]" % filename)
        else:
            if strict:
                self.err(test_case, "File found [%s],"
                                    " while it should be missing!" % full_path.replace(self._sbuild_path, ""))
            else:
                self.wrn(test_case, "File found [%s], while it should be missing!"
                                    " But strict checking not requested, will continue without error" % filename)

    def assert_file(self, test_case: str, filename: str, strict: bool = True) -> None:
        """Assert that the file exists."""
        full_path = self.lookup_entries(filename)

        if full_path:
            self.ok(test_case, "Found file [%s]" % full_path.replace(self._sbuild_path, ""))
        else:
            if strict:
                self.err(test_case, "File not found [%s]" % filename)
            else:
                self.wrn(test_case, "File not found [%s],"
                                    " but strict checking not requested, will continue without error" % filename)

    def assert_non_empty(self, test_case: str, filename: str, strict: bool = True) -> None:
        """Assert that the file is not empty."""
        full_path = self.lookup_entries(filename)

        if full_path:
            size = self.entries[full_path]["st_size"]
            if size == 0:
                if strict:
                    self.err(test_case, "File empty [%s]" % filename)
                else:
                    self.wrn(test_case, "File empty [%s], but strict checking not requested,"
                                        " will continue without error" % filename)
            else:
                self.ok(test_case, "File seems to be non empty [%s]" % filename)
        else:
            if strict:
                self.err(test_case, "File not found [%s]" % filename)
            else:
                self.wrn(test_case, "File not found [%s],"
                                    " but strict checking not requested, will continue without error" % filename)

    def assert_grep_file(self, test_case: str, filename: str, content: str, strict: bool = True) -> None:
        """Assert that given content is present in the file."""
        full_path = self.lookup_entries(filename)

        if not full_path:
            if strict:
                self.err(test_case, "File not present [%s]" % filename)
            else:
                self.wrn(test_case, "File not present [%s],"
                                    " but strict checking not requested, will continue without error" % filename)
            return

        try:
            pattern = ".*%s.*" % content
            regex = re.compile(pattern)

            with self._open(full_path, "r") as file_:
                for line in file_:
                    assert isinstance(line, str)
                    if regex.match(line):
                        self.ok(test_case, "Found matching string [%s] in file [%s]" % (content, filename))
                        return
            if strict:
                self.err(test_case, "Pattern [%s] not found in [%s]" % (content, filename))
            else:
                self.wrn(test_case, "Pattern [%s] not found in [%s], but strict checking not requested,"
                                    " will continue without error" % (content, filename))
        except (IOError, TypeError, sre_constants.error):
            if strict:
                self.err(test_case, "File not found [%s]" % filename)
            else:
                self.wrn(test_case, "File not found [%s],"
                                    " but strict checking not requested, will continue without error" % filename)


class Environment(ConfigAbstract):
    """Validator for XML environment file."""

    def __init__(self, sbuild_path: str, path: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(Environment, self).__init__(sbuild_path, path, name, status)

        tree = ElementTree.parse(os.path.join(sbuild_path, path))
        root = tree.getroot()

        items = root.findall(".//arrayref/item/arrayref/item")

        for item in items:
            assert item.text
            keyvalue = item.text.split("=", 1)
            self.entries[keyvalue[0]] = keyvalue[1]

    def get_var(self, name: str) -> str:
        """Get variable from the instance."""
        try:
            return os.environ[name]
        except KeyError:
            value = self.entries.get(name)
            if value:
                assert isinstance(value, str)
                return value
            return "UNKNOWN"

    def __str__(self) -> str:
        """Stringify as empty string."""
        return ""


class FlashMap(ConfigAbstract):
    """Validator for map of flash memory."""

    def __init__(self, sbuild_path: str, path: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(FlashMap, self).__init__(sbuild_path, path, name, status)

        with open(os.path.join(sbuild_path, path), "r") as file_:
            for line in file_:
                line_stripped = " ".join(line.split())
                if not line_stripped.startswith("#"):
                    split = line_stripped.split(" ")
                    # TO DO: partition names may repeat themselves, do something about it
                    if len(split) >= 5:
                        self.append(split)

    def append(self, csv: List[str]) -> None:
        """Append to entries after formatting as dictionary."""
        entry = {"offset": csv[0],
                 "size": csv[1],
                 "src": csv[2],
                 "dst": csv[3]}  # type: Dict[str, Any]

        try:
            entry["attrs"] = csv[5].split(",")
        except IndexError:
            entry["attrs"] = []

        self.entries[csv[4]] = entry

    def __str__(self) -> str:
        """Stringify as list of entries."""
        return ''.join(str(fme) + '\n' for fme in self.entries)


class Config(ConfigAbstract):
    """Configuration validator."""

    def __init__(self, sbuild_path: str, path: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(Config, self).__init__(sbuild_path, path, name, status)

        with open(os.path.join(sbuild_path, path), "r") as file_:
            for line in file_:
                line_stripped = line.strip()
                if line_stripped.startswith("#"):
                    # It's either a comment or a commented config option, need to check it out
                    comment_line = line_stripped.strip("#").strip()

                    if comment_line.startswith("CONFIG_"):
                        config_opt = comment_line.split(" ")
                        self._append_entry(config_opt[0], None)

                elif line_stripped.startswith("CONFIG_"):
                    # Now we have an active option, need to parse it
                    keyvalue = line_stripped.split("=", 1)
                    self._append_entry(keyvalue[0], re.sub(r'"(.*)"', r'\1', keyvalue[1]))

    def __str__(self) -> str:
        """Stringify as "mapping" from keys to values."""
        return ''.join("%s -> [%s]\n" % (ce, self.entries[ce]) for ce in self.entries)

    def run_checks(self, blacklist: List[str], exceptions: Optional[List[str]] = None,
                   must_haves: Optional[List[str]] = None) -> None:
        """Run checks using provided limitations on keys."""
        if exceptions is None:
            exceptions = []
        if must_haves is None:
            must_haves = []
        clist = [key for key in self.entries if self.entries[key] is not None]

        # Whitelist
        for w_entry in must_haves:
            if w_entry not in clist:
                self.err(w_entry, "MISSING required option: [%s]" % w_entry)
            else:
                self.ok(w_entry, "Required option found: [%s]" % w_entry)

        # Blacklist
        for b_entry in blacklist:
            for entry in self.entries:
                if re.fullmatch(b_entry, entry) and self.entries[entry] is not None and entry not in exceptions:
                    self.err(b_entry, "Illegal option: [%s] matching blacklist at [%s]" % (entry, b_entry))
                    break


class AppConfig(ConfigAbstract):
    """Application configuration validator."""

    def __init__(self, sbuild_path: str, path: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(AppConfig, self).__init__(sbuild_path, path, name, status)

        raw = open(os.path.join(sbuild_path, path), "r").read()
        entries = json.loads(raw)

        self._tree_to_serial(entries, self.entries, "")

    def _tree_to_serial(self, tree: Dict[str, Any], serial: Dict[str, Any], path: str) -> None:
        for key in tree:
            if isinstance(tree[key], dict):
                self._tree_to_serial(tree[key], serial, path + key + ".")
            else:
                serial[str(path + key)] = tree[key]

    def __str__(self) -> str:
        """Stringify into JSON string."""
        return json.dumps(self.entries, sort_keys=True, indent=4)


class Cert(ConfigAbstract):
    """Certificate validator."""

    def __init__(self, sbuild_path: str, _: str, name: str, status: GlobalStatus) -> None:
        """Initialize instance."""
        super(Cert, self).__init__(sbuild_path, '', name, status)
        self.certs = []  # type: List[X509.X509]

    def _generic_assert_output(self, test_case: str, condition: bool, entry: str,  # pylint: disable=too-many-arguments
                               value: Any, regex: Any, strict: bool) -> None:
        if condition:
            self.ok(test_case, "Variable [%s] -> [%s] checks out against [%s]" % (entry, value, regex))
        else:
            if strict:
                self.err(test_case, "Illegal variable value [%s] -> [%s] :"
                                    " does not match pattern [%s]" % (entry, value, regex))
            else:
                self.wrn(test_case, "Illegal variable value [%s] -> [%s] : does not match pattern [%s],"
                                    " but strict check is disabled" % (entry, value, regex))

    def load_cert_file(self, test_case: str, path: str,
                       strict: bool = True) -> bool:
        """Load certificate file and report, if this was a success."""
        try:
            # Amend the path to allow matching it in the host file system,
            # since the certs in /etc/ssl/certs are symbolic links to the files in
            # /etc/certs directory as they are seen from the target file system.
            if path.startswith("target/root/etc/ssl/certs/"):
                path = os.path.join(self._sbuild_path, 'target', 'root') \
                    + os.readlink(os.path.join(self._sbuild_path + path))
            else:
                path = os.path.join(self._sbuild_path + path)
            data = self._open(path, "rb").read()
            assert isinstance(data, bytes)
        except IOError:
            if strict:
                self.err(test_case, "File not found [%s]" % path)
            else:
                self.wrn(test_case, "File not found [%s], but strict check is disabled" % path)
            return False

        bio = BIO.MemoryBuffer(data)

        try:
            while True:
                # Read one cert at a time, 'bio' stores the last location read
                # Exception is raised when no more cert data is available
                cert = X509.load_cert_bio(bio)
                if not cert:
                    break
                self.certs.append(cert)

        except X509.X509Error:
            # This is the normal return path.
            # X509.load_cert_bio will throw an exception after reading all of the cert data
            pass

        if not self.certs:
            if strict:
                self.err(test_case, "File [%s] does not contain any X.509 certificates" % path)
            else:
                self.wrn(test_case, "File [%s] does not contain any X.509 certificates,"
                                    " but strict check is disabled" % path)
            return False
        return True

    def _assert_entry_value_internal(self, test_case: str, entry: str, value: str,  # pylint: disable=too-many-arguments
                                     regex: str, strict: bool) -> None:
        condition = re.match(regex, value)
        self._generic_assert_output(test_case, bool(condition), entry, value, regex, strict)

    def assert_issuer(self, test_case: str, regex: str, strict: bool = True) -> None:
        """Assert that certificates have the given issuer."""
        for idx, cert in enumerate(self.certs):
            ctc = "%s:%s" % (test_case, idx)
            value = self._get_cn(str(cert.get_issuer()))
            entry = "issuer"
            self._assert_entry_value_internal(ctc, entry, value, regex, strict)

    def assert_subject(self, test_case: str, regex: str, strict: bool = True) -> None:
        """Assert that certificates have the given subject."""
        for idx, cert in enumerate(self.certs):
            ctc = "%s:%s" % (test_case, idx)
            value = self._get_cn(str(cert.get_subject()))
            entry = "subject"
            self._assert_entry_value_internal(ctc, entry, value, regex, strict)

    def assert_not_before(self, test_case: str, expr: Callable[[datetime.datetime], bool], strict: bool = True) -> None:
        """Assert that certificates are not before the time."""
        for idx, cert in enumerate(self.certs):
            ctc = "%s:%s" % (test_case, idx)
            value = cert.get_not_before().get_datetime()
            # noinspection PyTypeChecker
            result = expr(value)  # we are mixing private datetime with the standard one
            entry = "not_before"
            self._generic_assert_output(ctc, result, entry, value, expr, strict)

    def assert_not_after(self, test_case: str, expr: Callable[[datetime.datetime], bool], strict: bool = True) -> None:
        """Assert that certificates are not after the time."""
        for idx, cert in enumerate(self.certs):
            ctc = "%s:%s" % (test_case, idx)
            value = cert.get_not_after().get_datetime()
            # noinspection PyTypeChecker
            result = expr(value)  # we are mixing private datetime with the standard one
            entry = "not_after"
            self._generic_assert_output(ctc, result, entry, value, expr, strict)

    @staticmethod
    def _get_cn(subject: str) -> str:
        match = re.match(".*CN=(.*)", subject)
        assert match
        return match.group(1)

    def __str__(self) -> str:
        """Stringify certificates into a simple list with basic information."""
        ret = ""
        for cert in self.certs:
            ret = ret + "[%s] <- [%s], [%s] : [%s]\n" % (
                self._get_cn(str(cert.get_subject())),
                self._get_cn(str(cert.get_issuer())),
                cert.get_not_before(),
                cert.get_not_after())
        return ret
