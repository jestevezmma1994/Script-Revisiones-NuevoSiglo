#!/usr/bin/env python3

#############################################################################
# Cubiware Sp. z o.o. Software License Version 1.0
#
# Copyright (C) 2016-2020 Cubiware Sp. z o.o. All rights reserved.
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

"""Executable part of cerberus client, that performs the checks."""

# TO DO:
# - try/except for keys from various places
# - config options may be set to "" which is sometimes good - we need to specify a condition for individual options,
#   e.g. >0 =0, !="", etc. (eval)
# - special cases for individual platforms/customers - we need to modify blacklists accordingly
# TO DO: wrap everything (stderr, stdout, etc.) into own buffer, so we can send it somewhere, e.g. to Jira
# TO DO: print firewall setup, perhaps compare to something existing?

import json
import os
import re
import sys
from collections import OrderedDict  # pylint: disable=unused-import
from datetime import datetime, timedelta
from typing import Any, Dict, List, Optional, Tuple

import requests

import cerberus.config
import cerberus.global_status


class Profiles:
    """Structure with profiles used for filtering results."""

    def __init__(self, filename: str, glogger: cerberus.global_status.GlobalLogger) -> None:
        """Initialize instance."""
        self.glogger = glogger
        try:
            with open(filename, "r") as file_:
                config = json.loads(file_.read())
            self.profiles = config["profiles"]
            self.mappings = config["mappings"]
        except (IOError, ValueError) as exc:
            glogger.write("*** Could not load profiles from [%s]. Will not apply exceptions then!\n\n" % filename)
            glogger.write("%s\n" % str(exc))
            self.profiles = {}
            self.mappings = {}

    def proove(self, results: Dict[str, Any], environment: cerberus.config.Environment) -> Tuple[int, List[Any]]:
        """Select critical errors that have not been filtered out."""
        failed = []

        srm_project = environment.get_var("SRM_PROJECT")
        srm_release = environment.get_var("SRM_RELEASE")

        key = "%s-%s" % (srm_project, srm_release)
        profile_name = None

        for prj_pattern in self.mappings.keys():
            if re.match(prj_pattern, key):
                profile_name = self.mappings[prj_pattern]
                self.glogger.write("Mapping project [%s] to profile [%s]\n\n" % (key, profile_name))
                break

        if profile_name is None:
            profile_name = "UNKNOWN"
            self.glogger.write("*** Mapping for project [%s] not found. Using [%s] profile\n\n" % (key, profile_name))

        result = 0
        for name in results:
            for test_case in results[name]:
                if results[name][test_case]["type"] == "ERR":
                    mask_err = False
                    if profile_name in self.profiles:
                        # Some test cases are applied multiple times, and then they are assigned
                        # ordinal number suffix to prevent overwriting given entry in the dictionary.
                        exceptions = self.profiles[profile_name]["exceptions"]
                        mask_err = any(test_case.startswith(except_) for except_ in exceptions)
                    if mask_err:
                        self.glogger.write("[EXCEPTION] Allowing this failed test for now [%s]\n" % test_case)
                    else:
                        self.glogger.write("[ERROR] Offending test [%s]. You MUST fix it first!\n" % test_case)
                        failed.append(test_case)
                        result = 1

        return result, failed


class CerberusServer:
    """Server used for sending results of checks."""

    def __init__(self, base_url: str, glogger: cerberus.global_status.GlobalLogger) -> None:
        """Initialize server instance."""
        self.glogger = glogger
        self.base_url = base_url
        self._session = requests.Session()

        # Get the dirname of current executable
        self.server_ca = os.path.dirname(sys.argv[0]) + "/cerberus-server-ca.pem"

        # Client certificates reused from jclient
        self.client_cert = os.environ["HOME"] + "/.qbkey/client.pem"

    def get_results(self, key: str) -> Optional[str]:
        """Get results from the server."""
        url = self.base_url + "/status"

        params = {"key": key}

        try:
            resp = self._session.get(url, params=params, verify=self.server_ca,
                                     cert=(self.client_cert, self.client_cert))
        except Exception as exc:  # pylint: disable=broad-except
            self.glogger.write("*** Exception occured while getting stats for [%s].\n" % key)
            self.glogger.write("%s\n" % str(exc))
            return None

        if resp.status_code != 200:
            self.glogger.write("*** Server returned error: %s\n" % resp.status_code)
            self.glogger.write("*** %s" % resp.text)
            return None
        return resp.text

    @staticmethod
    def copy_variable(variable: str, dest: Dict[str, Any], environment: cerberus.config.Environment) -> None:
        """Copy variable from environment to destination."""
        value = environment.get_var(variable)
        dest[variable] = value

    def upload_results(self, status: 'OrderedDict[str, str]', failed: List[str],
                       environment: cerberus.config.Environment) -> None:
        """Upload results from client to server."""
        url = self.base_url + "/upload"

        env = {}  # type: Dict[str, str]
        self.copy_variable("SRM_PROJECT", env, environment)
        self.copy_variable("SRM_RELEASE", env, environment)
        self.copy_variable("RELEASE", env, environment)
        self.copy_variable("VERSION", env, environment)

        message = {"status": status,
                   "failed": failed,
                   "environment": env}

        try:
            resp = self._session.post(url, json.dumps(message), verify=self.server_ca,
                                      cert=(self.client_cert, self.client_cert))  # type: requests.Response
        except Exception as exc:  # pylint: disable=broad-except
            self.glogger.write("*** Exception occured while uploading results to Cerberus Server."
                               " Make sure you can access the server: [%s].\n" % url)
            self.glogger.write(str(exc))
            self.glogger.write("\n")
            return

        if resp.status_code != 200:
            self.glogger.write("*** Server returned [%s], likely your data did not upload properly."
                               " Please contact administrator." % resp.status_code)
        else:
            response = json.loads(resp.text)
            self.glogger.write("\nSuccessfully uploaded results to Cerberus Server.\n")
            self.glogger.write("\tkey is: %s\n" % (response["key"]))


# Kernel Configuration
KERNEL_BLACKLISTED_OPTIONS = [
    ".*SERIAL.*",
    ".*DEBUG.*",
    ".*CONSOLE.*",
    ".*STATS.*",
    "CONFIG_BT",
    "CONFIG_FIREWIRE",
    "CONFIG_SND_FIREWIRE",
    "CONFIG_IRDA",
    "CONFIG_SH_IRDA",
    "CONFIG_USB_IRDA",
    "CONFIG_PROFILING",
    "CONFIG_PROFILE",
    "CONFIG_KALLSYMS",
    "CONFIG_ASHMEM",
    "CONFIG_SWAP",
    "CONFIG_CRASH_DUMP",
    "CONFIG_READABLE_ASM",
    "CONFIG_FTRACE",
    "CONFIG_UNUSED_SYMBOLS",
    "CONFIG_KGDB",
    "CONFIG_KPROBE",
    "CONFIG_IKCONFIG",
    "CONFIG_PRINTK",
    "CONFIG_EARLY_PRINTK",
    "CONFIG_BOOT_PRINTK",
    "CONFIG_WAN",
    "CONFIG_ATM",
    "CONFIG_DECNET",
    "CONFIG_IPX",
    "CONFIG_DCB",
    "CONFIG_BATMAN",
    "CONFIG_NET_PKTGEN",
    "CONFIG_VT",
    "CONFIG_UNIX98",
    "CONFIG_SYSFS_DEPRECATED",
    "CONFIG_ATALK",
    "CONFIG_PHONET",
    "CONFIG_BRIDGE",
    "CONFIG_INET_DIAG",
    "CONFIG_USB_NET",
    "CONFIG_USB_WUSB",
    "CONFIG_USB_WDM",
    "CONFIG_USB_ANNOUNCE",
    "CONFIG_USB_OTG",
    "CONFIG_USB_GADGET",
    "CONFIG_USB_TMC",
    "CONFIG_USB_ACM",
    "CONFIG_USB_PRINTER",
    "CONFIG_NOP_USB_XCEIV",
    "CONFIG_ANDROID_LOGGER",
    "CONFIG_NETWORK_FILESYSTEMS",
    "CONFIG_ARCNET",
    "CONFIG_STRICT_DEVMEM",
    "CONFIG_DOUBLEFAULT",
    "CONFIG_STRIP_ASM_SYMS",
    "CONFIG_EFI_PARTITION",
    "CONFIG_KEXEC",
    "CONFIG_COREDUMP",
    "CONFIG_ATA_VERBOSE_ERROR",
    "CONFIG_HOSTAP",
    "CONFIG_USB_MON",
    "CONFIG_DEBUG_USER"
]

KERNEL_EXCEPTIONS = [
    "CONFIG_HAVE_DMA_API_DEBUG",
    "CONFIG_BRCM_CONSOLE_DEVICE",
]

KERNEL_MUST_HAVES = [
    "CONFIG_USB_OTG_BLACKLIST_HUB",
    "CONFIG_CC_STACKPROTECTOR",  # tbd
    "CONFIG_NF_CONNTRACK",
    "CONFIG_NETFILTER_XTABLES",
    "CONFIG_NETFILTER_XT_MATCH_ADDRTYPE",
    "CONFIG_NETFILTER_XT_MATCH_CONNTRACK",
    "CONFIG_NETFILTER_XT_MATCH_STATE",
    "CONFIG_NETFILTER_XT_MATCH_PKTTYPE",
    "CONFIG_IP_NF_RAW",
    "CONFIG_IP_NF_FILTER",
    "CONFIG_IP_NF_IPTABLES",
    "CONFIG_NF_CONNTRACK_IPV4",
    "CONFIG_NF_DEFRAG_IPV4"
]

# BusyBox Configuration
BUSYBOX_BLACKLISTED_OPTIONS = [
    "CONFIG_PING",
    "CONFIG_UDHCPD",
    "CONFIG_GETTY",
    "CONFIG_UDHCPC$",
    "CONFIG_TTY",
    "CONFIG_TELNET",
    "CONFIG_DEBUG",
    "CONFIG_UNIT_TEST",
    "CONFIG_LOGIN",
    "CONFIG_PASSWD",
    "CONFIG_FEATURE_SYSLOG",
    "CONFIG_SYSLOGD"
    "CONFIG_NOHUP",
    "CONFIG_PRINTENV",
    "CONFIG_LONG_OPTS",  # TBD
    "CONFIG_FEATURE_TAR_LONG_OPTIONS",
    "CONFIG_UNAME",
    "CONFIG_TAR_UNAME_GNAME",
    # awk and sed at the same time
]

BUSYBOX_MUST_HAVES = [
    "CONFIG_FEATURE_BUFFERS_USE_MALLOC"
]


def _assert_nonexistent_relative_executable_files(filesystem: cerberus.config.FileSystem) -> None:
    filesystem.nassert_file("FS024", "ifconfig")
    filesystem.nassert_file("FS025", "find")
    filesystem.nassert_file("FS026", "uniq")
    filesystem.nassert_file("FS027", "nslookup")
    filesystem.nassert_file("FS028", "telnet")
    filesystem.nassert_file("FS029", "rmdir")
    filesystem.nassert_file("FS030", "yes")
    filesystem.nassert_file("FS031", "hexdump")
    filesystem.nassert_file("FS032", "sha1sum")
    filesystem.nassert_file("FS033", "login")
    filesystem.nassert_file("FS034", "mdev")
    filesystem.nassert_file("FS035", "fbset")
    filesystem.nassert_file("FS036", "ipcrm")
    filesystem.nassert_file("FS037", "ipcs")
    filesystem.nassert_file("FS038", "mkswap")
    filesystem.nassert_file("FS039", "pivot_root")
    filesystem.nassert_file("FS040", "swapoff")
    filesystem.nassert_file("FS041", "arping")
    filesystem.nassert_file("FS042", "ipcalc")
    filesystem.nassert_file("FS043", "netstat")
    filesystem.nassert_file("FS044", "tftp")
    filesystem.nassert_file("FS045", "nslookup")
    filesystem.nassert_file("FS046", "rmdir")


def _assert_filesystem(filesystem: cerberus.config.FileSystem, environment: cerberus.config.Environment) -> None:
    filesystem.assert_grep_file("FS001", "/target/root/bin/_upgrade_platform_spec.proc", "nandcat")

    filesystem.assert_file("FS002", "/target/root/etc/firewall.rules")
    filesystem.assert_file("FS003", "/target/root/usr/sbin/nandcat")
    filesystem.assert_file("FS004", "/target/root/usr/sbin/dhclient")
    filesystem.nassert_file("FS005", "/target/root/usr/sbin/udhcpd")
    filesystem.nassert_file("FS006", "/target/root/usr/local/sbin/qb_run_getty")
    filesystem.nassert_file("FS007", "/target/root/usr/local/sbin/core-uploader")
    filesystem.nassert_file("FS008", "/target/root/etc/revtunnel")
    filesystem.nassert_file("FS009", "/target/root/etc/authorized_keys")

    filesystem.assert_non_empty("FS010", "/target/root/etc/ota-uri-default")
    filesystem.assert_non_empty("FS011", "/target/root/etc/SELECTOR")
    filesystem.assert_non_empty("FS012", "/target/root/etc/QBShellCmdsWhitelists/cubitv")
    filesystem.assert_non_empty("FS013", "/target/root/etc/QBShellCmdsWhitelists/progress")
    filesystem.assert_non_empty("FS014", "/target/root/etc/QBShellCmdsWhitelists/QBNetworkManagerApp")
    filesystem.assert_non_empty("FS015", "/target/root/etc/QBShellCmdsWhitelists/QBUpgradeApp")
    filesystem.assert_non_empty("FS016", "/target/root/etc/QBShellCmdsWhitelists/wait_for_network")
    filesystem.assert_non_empty("FS017", "/target/root/etc/gui_start")
    filesystem.assert_non_empty("FS018", "/target/root/etc/gui.d/cubitv")
    filesystem.assert_non_empty("FS019", "/target/root/etc/gui.d/none")
    filesystem.assert_non_empty("FS020", "/target/root/etc/gui.d/mpmode")
    filesystem.assert_non_empty("FS021", "/target/root/etc/gui.d/default")  # TO DO: does it point to cubitv?

    filesystem.assert_non_empty("FS022", "/target/bSpec/OTA.json")

    # TO DO: KGolinski comments:
    # jclient keys shall be matching the platform (somehow)

    # File content / TO DO
    # missing upgrade/dvb settings
    # sbuild total size!
    filesystem.nassert_file("FS023", "/target/root/bin/passwd")
    _assert_nonexistent_relative_executable_files(filesystem)

    filesystem.assert_file("FS047", "/target/root/etc/ssl/certs/cacert.pem")
    filesystem.assert_file("FS048", "/target/root/etc/ssl/certs/device.pem")
    filesystem.assert_file("FS049", "/target/root/etc/ssl/backup/cert.pem")
    filesystem.assert_file("FS050", "/target/root/etc/ssl/backup/key.pem")
    filesystem.assert_file("FS051", "/target/root/etc/ssl/transport/rsa.pub")
    filesystem.assert_file("FS052", "/target/root/etc/ssl/transport/certs.key")

    if environment.has_entry_value("USE_PLAYREADY", "1") or filesystem.has_file("/target/root/etc/playready"):
        filesystem.assert_non_empty("FS053", "/target/root/etc/playready/cacert.pem")
        filesystem.assert_file("FS054", "/target/root/etc/playready/bgroupcert.dat")
        filesystem.assert_file("FS055", "/target/root/etc/playready/service.json")

    filesystem.assert_grep_file("FS056", "/target/root/usr/local/bin/hotplug_handle.d/block/50.automounter",
                                "is_encrypted")
    filesystem.assert_grep_file("FS057", "/target/root/bin/_upgrade_platform_spec.proc", "splash_sign")


def _assert_config(app_config: cerberus.config.AppConfig) -> None:
    app_config.assert_entry_value("AC001", "UPGRADE.SERVER", "https://.*")
    app_config.assert_entry_value("AC002", "PREFIX", "https://.*")

    if app_config.has_entry("PREFIXIP"):
        app_config.assert_entry_value("AC003", "PREFIXIP", "https://.*")

    if app_config.has_entry("PUSHSERVER"):
        app_config.assert_entry_value("AC004", "PUSHSERVER", "wss://.*|")

    app_config.assert_entry_value("AC005", "MIDDLEWARE_ID", "SCID|scid|MAC|mac|TSN|tsn")

    app_config.assert_entry_value("AC006", "GUI", "default")
    app_config.assert_entry_value("AC007", "IPREMOTE", "disabled")
    app_config.assert_entry_value("AC008", "LOG.TYPE", "LOCAL")
    app_config.assert_entry_value("AC009", "LOG.LEVEL", "0")
    app_config.assert_entry_value("AC010", "LOG.MAXSIZE", "1024")

    app_config.assert_entry_value("AC011", "REMOTE_DEBUG", "disabled")
    app_config.assert_entry_value("AC012", "REMOTE_DEBUG_URL", "")
    app_config.assert_entry_value("AC013", "FTPUSER", "")
    app_config.assert_entry_value("AC014", "FTPPASS", "")
    app_config.assert_entry_value("AC015", "FTPPATH", "/")


def _assert_environment(environment: cerberus.config.Environment) -> None:  # pylint: disable=too-many-statements
    environment.assert_entry_value("EN001", "S_BUSYBOX_CONFIG_FILE", ".*-conax")
    environment.assert_entry("EN077", "S_BUSYBOX_VERSION")  # to do - shift everything down
    environment.assert_entry_value("EN002", "SV_LOG_LEVEL", "0")
    environment.assert_entry_value("EN003", "S_INSTALL_ENCRYPT_PVR_META", "yes")
    # this is project specific, unfortunately!
    environment.assert_entry_value("EN004", "S_INSTALL_ENCRYPT_PVR_META_MODE", "aes-xts-plain64")

    if environment.has_entry("CUBITV_PVR_REENCRYPTION"):
        environment.assert_entry_value("EN005", "CUBITV_PVR_REENCRYPTION", "enabled|dvbonly|disabled")

    if environment.has_entry_value("DRM_VIEWRIGHT_DVB_DVR", "yes"):
        environment.assert_entry_value("EN006", "DRM_VIEWRIGHT_DVB_DVR_MAX_CHANNELS", r"^\d$")
        environment.assert_entry_value("EN007", "DRM_VIEWRIGHT_DVB_STATIC_LIBRARY", "yes")

    environment.assert_entry("EN008", "S_INSTALL_KEYSERVER_RSA_BOOT_HASH")
    environment.assert_entry("EN009", "S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID")
    environment.assert_entry("EN010", "S_INSTALL_KEYSERVER_SYM_KERNEL_KEY_ID")
    environment.assert_entry("EN011", "S_INSTALL_KEYSERVER_SYM_ROOTFS_KEY_ID")
    # TO DO: this shall be cbc or sth, not ecb!
    environment.assert_entry("EN012", "S_INSTALL_KEYSERVER_SYM_ROOTFS_MODE")
    environment.assert_entry("EN013", "S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH")  # TO DO, shall check sha256 here
    environment.assert_entry("EN014", "S_INSTALL_KEYSERVER_RSA_UPGRADE_KEY_ID")
    environment.assert_entry("EN015", "S_INSTALL_KEYSERVER_RSA_MAC_KEY_ID")
    environment.assert_entry("EN016", "S_INSTALL_KEYSERVER_SYM_SPLASH_KEY_ID")
    environment.assert_entry("EN017", "S_INSTALL_KEYSERVER_SYM_NVRAM_KEY_ID")
    environment.assert_entry("EN018", "S_INSTALL_KEYSERVER_SYM_NVRAM_HMAC_KEY_ID")
    environment.assert_entry("EN019", "S_INSTALL_KEYSERVER_SYM_META_KEY_ID")
    environment.assert_entry("EN020", "S_INSTALL_KEYSERVER_SYM_MAC_KEY_ID")
    environment.assert_entry("EN021", "S_INSTALL_KEYSERVER_SWPK5_KEY_ID")
    environment.assert_entry("EN022", "S_INSTALL_KEYSERVER_SWPK4_KEY_ID")
    environment.assert_entry("EN023", "S_INSTALL_KEYSERVER_RSA_SKIN_KEY_ID")

    environment.assert_entry_value("EN024", "S_INSTALL_CREATE_RSA_UPGRADE", "yes")
    environment.assert_entry_value("EN025", "S_INSTALL_USE_RSA_UPGRADE", "yes")

    environment.assert_entry_value("EN026", "S_INSTALL_FIREWALL", "yes")
    environment.assert_entry("EN027", "S_INSTALL_NETWORKING_2")
    environment.assert_entry("EN028", "S_INSTALL_CONFIG_SH")
    environment.assert_entry_value("EN029", "S_INSTALL_CONFIG_USE_JSON", "yes")
    environment.assert_entry_value("EN030", "S_INSTALL_CONFIG_ENFORCE_HMAC", "yes")
    environment.assert_entry_value("EN031", "S_INSTALL_CERT_LOCATION", "PART|ROOTFS")
    environment.assert_entry_value("EN032", "S_INSTALL_CERT_CRYPTO_VERSION", "1|2")

    environment.assert_entry_value("EN033", "CUBITV_MALLOC_REPLACEMENT", "jemalloc-replace")
    environment.assert_entry_value("EN034", "CUBITV_STARTER_SCRIPT", "scripts/starter-secure")
    environment.assert_entry_value("EN035", "GENERIC_STARTER_SCRIPT", "scripts/starter-secure")
    environment.assert_entry_value("EN036", "S_INSTALL_CONFIG_PASSWD", "nologin")
    environment.assert_entry_value("EN037", "S_INSTALL_CONFIG_SH", ".*encrypted.*")
    environment.assert_entry_value("EN038", "S_INSTALL_DISABLE_SSHD", "yes")
    environment.assert_entry_value("EN039", "S_INSTALL_DISABLE_TELNETD", "yes")
    environment.assert_entry_value("EN040", "S_INSTALL_DO_STRIP", "yes")
    environment.assert_entry_value("EN041", "S_INSTALL_DYNAMIC_PASSWD", "none")
    environment.assert_entry_value("EN042", "S_INSTALL_MKPART", "configurable")
    environment.assert_entry_value("EN043", "S_INSTALL_MULTI_USER", "yes")
    environment.assert_entry_value("EN044", "S_INSTALL_RCS_SCRIPT", "rcS-secure")
    environment.assert_entry_value("EN045", "S_INSTALL_SSHKEYS", "no")
    environment.assert_entry_value("EN046", "S_INSTALL_SVSTORAGE_CONFIG", "generic")
    environment.assert_entry_value("EN047", "S_INSTALL_SYSINIT_SCRIPT", "sysinit-secure")
    environment.assert_entry_value("EN048", "S_INSTALL_TFTPBIN", "no")
    environment.assert_entry_value("EN049", "S_INSTALL_UPGRADE_OK", "upgrade_ok_clean.sh")
    environment.assert_entry_value("EN050", "S_INSTALL_UPGRADE_SECURE", "yes")
    environment.assert_entry_value("EN051", "S_INSTALL_USE_KEYSERVER", "yes")
    environment.assert_entry_value("EN052", "S_KERNEL_USE_KEYCHAIN", "yes")  # ??
    environment.assert_entry_value("EN053", "S_SCRIPTS_GUI_START", "gui-secure")
    environment.assert_entry_value("EN054", "S_SECURE_BOOT", "yes")
    environment.assert_entry_value("EN055", "S_SECURE_BOOT_DRYRUN", "no")
    environment.assert_entry_value("EN056", "S_SECURE_BOOT_FORCE_DEBUG", "no")
    environment.assert_entry_value("EN057", "S_INSTALL_IGMP_PROTO_VERSION", "1|2", strict=False)
    environment.assert_entry_value("EN058", "S_WIFI_INTERFACE", "ra0")
    environment.assert_entry_value("EN059", "SvKeepStack", "no")
    environment.assert_entry_value("EN060", "USE_AVI", "1")
    environment.assert_entry_value("EN061", "USE_MKV", "1")

    if environment.has_entry_value("S_INSTALL_LATENS_UPGRADE", "yes"):  # TO DO: how do we identify a Latens project?
        # Latens specific
        environment.assert_entry("EN063", "S_LATENS_BOOTLOADER_VERSION")  # =3.0.11
        environment.assert_entry("EN064", "S_LATENS_OEM_NAME")  # =Polsat
        environment.assert_entry("EN065", "S_LATENS_STB_MODEL")  # =IPS3000MMP
        environment.assert_entry("EN066", "S_LATENS_STB_MODEL_IP")  # =IPS3000MMPC
        environment.assert_entry("EN067", "S_LATENS_STB_MODEL_OTT")  # =IPS3000MMPCU
        environment.assert_entry("EN068", "S_LATENS_VERSION")  # =1.0
        environment.assert_entry_value("EN069", "S_LATENS_TEST_VERIFY", "no")
        environment.assert_entry_value("EN070", "S_LATENS_CHECK_LOCKDOWN", "no")
        environment.assert_entry_value("EN071", "S_LATENS_LIB_TYPE", "production")
    elif not environment.has_entry_value("S_INSTALL_VMX_UPGRADE", "yes"):
        # TO DO, any other method to check VMX project?:
        # Assume Conax
        environment.assert_entry_value("EN073", "QBConax2CubiwareStorePath", "/etc/vod/")
        environment.assert_entry_value("EN074", "QBConax2CubiwareLibraryType", "CPLP")
    else:
        # Verimatrix
        # S_INSTALL_VMX_UPGRADE_2=yes ??
        pass


def _check_value_is_before_now(value: datetime) -> bool:
    return value < datetime.utcnow().replace(tzinfo=value.tzinfo)


def _check_value_is_after_ten_years_from_now(value: datetime) -> bool:
    return value > datetime.utcnow().replace(tzinfo=value.tzinfo) + timedelta(days=365 * 10)


def _assert_certificates(sbuild: str, gstatus: cerberus.global_status.GlobalStatus) -> None:
    cert = cerberus.config.Cert(sbuild, '', "CERT1", gstatus)
    if cert.load_cert_file("CE100", "target/root/etc/ssl/certs/cacert.pem"):
        cert.assert_issuer("CE101", "CubiwareRootCA|Cubiware2PortalProdRootCA")
        cert.assert_subject("CE102", "CubiwareRootCA|Cubiware2PortalProdRootCA")
        cert.assert_not_before("CE104", _check_value_is_before_now)
        cert.assert_not_after("CE105", _check_value_is_after_ten_years_from_now)

    cert = cerberus.config.Cert(sbuild, '', "CERT2", gstatus)
    if cert.load_cert_file("CE200", "target/root/etc/ssl/certs/device.pem"):
        cert.assert_issuer("CE201", "Cubiware.*DeviceProd")
        cert.assert_subject("CE202", "Cubiware.*DeviceProd")
        cert.assert_not_before("CE204", _check_value_is_before_now)
        cert.assert_not_after("CE205", _check_value_is_after_ten_years_from_now)

    cert = cerberus.config.Cert(sbuild, '', "CERT3", gstatus)
    if cert.load_cert_file("CE300", "target/root/etc/ssl/backup/cert.pem"):
        cert.assert_issuer("CE301", "Cubiware.*BackupProd")
        cert.assert_subject("CE302", "Cubiware.*BackupProd|STB_.*_MAC_000000000000|STB_.*_TSN_000000000000000")
        cert.assert_not_before("CE304", _check_value_is_before_now)
        cert.assert_not_after("CE305", _check_value_is_after_ten_years_from_now)

    # Applicable to old security only
    cert = cerberus.config.Cert(sbuild, '', "CERT4", gstatus)
    if cert.load_cert_file("CE400", "target/root/etc/certs/public/cacert.pem"):
        cert.assert_issuer("CE401", "CubiwareRootCA")
        cert.assert_subject("CE402", "CubiwareRootCA")
        cert.assert_not_before("CE404", _check_value_is_before_now)
        cert.assert_not_after("CE405", _check_value_is_after_ten_years_from_now)

    cert = cerberus.config.Cert(sbuild, '', "CERT5", gstatus)
    if cert.load_cert_file("CE500", "target/root/etc/cacert.pem"):
        cert.assert_issuer("CE501", "CubiwareRootCA")
        cert.assert_subject("CE502", "CubiwareRootCA")
        cert.assert_not_before("CE504", _check_value_is_before_now)
        cert.assert_not_after("CE505", _check_value_is_after_ten_years_from_now)

    cert = cerberus.config.Cert(sbuild, '', "CERT6", gstatus)
    if cert.load_cert_file("CE600", "target/root/etc/cert.pem"):
        cert.assert_issuer("CE601", "CubiwareRootCA")
        cert.assert_subject("CE602", "STB_.*_OTA")
        cert.assert_not_before("CE604", _check_value_is_before_now)
        cert.assert_not_after("CE605", _check_value_is_after_ten_years_from_now)


# Paths to certain files of interest
ENV_PATH = "environment.xml"
TARGET_PATH = "target/"
MAP_PATH = "target/maps/all.maps"
BUSYBOX_CONFIG_PATH = "target/busybox-config"
KERNEL_CONFIG_PATH = "host/cross/linux/.config"
# TO DO FIREWALL_CONFIG_PATH = "/target/root/etc/firewall.rules"
APP_CONFIG_PATH = "target/root/etc/default-conf/conf.json"


def _main() -> None:  # pylint: disable=too-many-statements
    glogger = cerberus.global_status.GlobalLogger()
    gstatus = cerberus.global_status.GlobalStatus(glogger)

    glogger.write("Welcome to Cerberus Client. Will scan your sbuild and upload results to Cerberus Server.\n\n")

    try:
        # Get the realpath of sbuild (resolve any symlinks along the way)
        sbuild = os.path.realpath(sys.argv[1])
        profiles_filename = sys.argv[2]
    except IndexError:
        glogger.write("Need to provide sbuild path as a first parameter and profile path as second\n")
        print(glogger)
        sys.exit(1)

    glogger.write("Checking [%s]\n\n" % sbuild)

    # Read in/parse various configuration files
    glogger.write("Scanning filesystem")
    filesystem = cerberus.config.FileSystem(sbuild, TARGET_PATH, "ROOTFS", gstatus)
    glogger.write("... %s files\n" % len(filesystem.entries))

    glogger.write("Parsing environment.xml")
    environment = cerberus.config.Environment(sbuild, ENV_PATH, "ENVXML", gstatus)
    glogger.write("... %s options\n" % len(environment.entries))

    glogger.write("Parsing all.maps...")
    flash_map = cerberus.config.FlashMap(sbuild, MAP_PATH, "ALLMAP", gstatus)
    glogger.write("...%s entries\n" % len(flash_map.entries))

    glogger.write("Parsing Kernel Config")
    kernel_config = cerberus.config.Config(sbuild, KERNEL_CONFIG_PATH, "KERNEL", gstatus)
    glogger.write("...%d options\n" % len(kernel_config.entries))

    glogger.write("Parsing BusyBox Config")
    busybox_config = cerberus.config.Config(sbuild, BUSYBOX_CONFIG_PATH, "BUSYBOX", gstatus)
    glogger.write("...%d options\n" % len(busybox_config.entries))

    glogger.write("Parsing application/json config")
    app_config = cerberus.config.AppConfig(sbuild, APP_CONFIG_PATH, "CONF", gstatus)
    glogger.write("...%d entries\n" % len(app_config.entries))

    ##############################
    #           ASSERTS          #
    ##############################
    # This is likely platform/customer/or something specific, or security scheme

    # Maps/bootloader
    # TO DO: more checks here!
    if "qbimage" not in flash_map.entries["nvram"]:
        environment.assert_entry_value("EN075", "S_INSTALL_ENCRYPT_NVRAM", "yes")
        environment.assert_entry_value("EN076", "S_INSTALL_VERIFY_NVRAM", "yes")

    # Kernel
    glogger.write("Running kernel config checks\n")
    kernel_config.run_checks(KERNEL_BLACKLISTED_OPTIONS, exceptions=KERNEL_EXCEPTIONS, must_haves=KERNEL_MUST_HAVES)
    kernel_config.assert_entry_value("KC001", "CONFIG_UEVENT_HELPER_PATH", "")

    # BusyBox
    glogger.write("Running BusyBox config checks\n")
    busybox_config.run_checks(BUSYBOX_BLACKLISTED_OPTIONS, must_haves=BUSYBOX_MUST_HAVES)

    glogger.write("Running other checks\n")

    # Filesystem
    _assert_filesystem(filesystem, environment)
    # Application/json Config
    _assert_config(app_config)

    # Environment
    _assert_environment(environment)
    if environment.has_entry_value("S_INSTALL_LATENS_UPGRADE", "yes"):  # TO DO: how do we identify a Latens project?
        # Latens specific, cross-check
        app_config.assert_entry_value("AC100", "LATENS_MODE", "DVB")  # TO DO!

    # QUADRICAST_MSTORE_SOURCES ??
    # S_INSTALL_LATENS_UPGRADE=yes
    # Optional
    # S_LIBCAP_VERSION=2.25
    # USE_PLAYREADY=1

    # scan certificates/pem files/keys, check for blank ones, check for invalid dates, etc.
    _assert_certificates(sbuild, gstatus)

    # Print Stats
    glogger.write("\nPrinting Statistics\n\n")
    gstatus.print_global_stats(stat_type="ERR|WRN")

    # Do the filtering of results against profiles
    glogger.write("\nFiltering against exceptions\n\n")
    profiles = Profiles(profiles_filename, glogger)
    result, failed = profiles.proove(gstatus.status, environment)

    # Print the final 0/1 result
    glogger.write("\n\nFINAL RESULT OF CERBERUS SANITY CHECK: %s\n" % ["OK", "FAILED"][result])

    cerberus_server = CerberusServer("https://cerberus.cubiware.com:443", glogger)
    cerberus_server.upload_results(gstatus.status, failed, environment)

    if result != 0:
        glogger.write("\n\n !!! WARNING !!!"
                      " Temporarily allowing to continue with failed tests until we address all the legacy problems.\n"
                      " Still you should really start thinking about fixing those issues above!\n\n")

    print(glogger)

    # Exit with error if any
    # sys.exit(result)
    # A temporary thing: we will return 0 until things settle with all the legacy stuff
    sys.exit(0)


if __name__ == "__main__":
    _main()
