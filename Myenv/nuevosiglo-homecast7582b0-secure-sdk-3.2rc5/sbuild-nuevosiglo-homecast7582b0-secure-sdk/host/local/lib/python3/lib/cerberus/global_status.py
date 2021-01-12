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

"""Module with status of cerberus as both wrapped dictionary and buffer-based logger."""

import re
from collections import OrderedDict
from typing import Any, Optional


class GlobalLogger:
    """Simple string buffer posing as logger."""

    def __init__(self) -> None:
        """Initialize instance."""
        self.text = ""

    def write(self, buf: str) -> None:
        """Write to logger instance."""
        self.text = self.text + buf

    def __str__(self) -> str:
        """Get content of logger."""
        return self.text


class GlobalStatus:
    """Status of cerberus."""

    def __init__(self, glogger: GlobalLogger) -> None:
        """Initialize instance."""
        self.glogger = glogger
        self.status = OrderedDict()  # type: OrderedDict[str, Any]

    def update_status(self, test_case: str, name: str, stat_type: str, msg: str) -> None:
        """Add information about a test case to status."""
        if name not in self.status:
            self.status[name] = OrderedDict()

        if test_case in self.status[name]:
            raise IndexError("This testcase [%s] has already been performed."
                             " Check your source code for duplicates!" % test_case)
        self.status[name][test_case] = {"type": stat_type, "msg": msg}

    def wrn(self, test_case: str, name: str, msg: str) -> None:
        """Add warning for a test case."""
        self.update_status(test_case, name, "WRN", msg)

    def err(self, test_case: str, name: str, msg: str) -> None:
        """Add error for a test case."""
        self.update_status(test_case, name, "ERR", msg)

    def ok(self, test_case: str, name: str, msg: str) -> None:  # pylint: disable=invalid-name
        """Add OK for a test case."""
        self.update_status(test_case, name, "OK", msg)

    def print_tc_status(self, test_case: str, name: str) -> None:
        """Print status of a test case."""
        if name in self.status and test_case in self.status[name]:
            tc_type = self.status[name][test_case]["type"]
            self.glogger.write("[%-3s] [%5s] %s\n" % (tc_type, test_case, self.status[name][test_case]["msg"]))

    def num_status(self, name: Optional[str], stat_type: str) -> int:
        """Numeric value of status for the given name."""
        if name is None:
            names = list(self.status.keys())
        elif name in self.status:
            names = [name]
        else:
            return 0
        return sum(len([tc_ for tc_ in self.status[name_] if self.status[name_][tc_]["type"] == stat_type])
                   for name_ in names)

    def print_global_stats(self, stat_type: str = ".*") -> None:
        """Print all statistics for given status regex."""
        global_ok = 0
        global_wrn = 0
        global_err = 0

        for name in self.status:
            self.print_stats(name, stat_type)
            global_ok = global_ok + self.num_status(name, "OK")
            global_wrn = global_wrn + self.num_status(name, "WRN")
            global_err = global_err + self.num_status(name, "ERR")

        # Print out global stats
        self.glogger.write("\nGlobal Statistics\n\n OK: %4s, WRN: %4s, ERR: %4s, Total: %s\n\n" % (
            global_ok, global_wrn, global_err,
            global_ok + global_wrn + global_err
        ))

    def print_stats(self, name: str, stat_type: str = ".*") -> None:
        """Print statistics about types given by regex."""
        nok = self.num_status(name, "OK")
        nwrn = self.num_status(name, "WRN")
        nerr = self.num_status(name, "ERR")

        self.glogger.write("%-20s: OK: %4s, WRN: %4s, ERR: %4s, Total: %4s\n" % (
            name,
            nok, nwrn, nerr,
            nok + nwrn + nerr
        ))

        regex = re.compile(stat_type)
        if name in self.status:
            for test_case in self.status[name]:
                if regex.match(self.status[name][test_case]["type"]):
                    self.print_tc_status(test_case, name)
        self.glogger.write("\n")
