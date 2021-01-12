#!/usr/bin/env python

"""*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************"""

from __future__ import print_function
import argparse
import array
import struct
import subprocess
import sys
import binascii
import re
import os
import string
import fileinput
import shutil
from stat import *

ROOTFS_SIZE = 230
CONFIG_SIZE = 200
KERNEL_SIZE = 50

class readDeviceAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs=None, **kwargs):
        if nargs is not None:
            raise ValueError("nargs not allowed")
        super(readDeviceAction, self).__init__(option_strings, dest, **kwargs)
    def __call__(self, parser, namespace, values, option_string=None):

        if re.match("^[A-Za-z]+$", values) is None:
            raise ValueError('Device name contain illegal signes: %s' % values)

        pathname = os.path.join('/dev', values)
        mode = os.stat(pathname)[ST_MODE]
        if not S_ISBLK(mode):
            raise ValueError('File has invalid type: %s. It should be block device.' % values)

        setattr(namespace, self.dest, values)

class verifyPathAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs=None, **kwargs):
        if nargs is not None:
            raise ValueError("nargs not allowed")
        super(verifyPathAction, self).__init__(option_strings, dest, **kwargs)
    def __call__(self, parser, namespace, values, option_string=None):

        mode = os.stat(values)[ST_MODE]
        if not S_ISDIR(mode):
            raise ValueError('Object has invalid type: %s. It should be directory.' % values)

        setattr(namespace, self.dest, values)

class InvalidPartitionError(Exception):
    pass

def findPartitions(dirPath, suffix):
    list = []
    for f in os.listdir(dirPath):
        if re.match(("%s[0-9]" % suffix), f):
            list.append(f)
    list.reverse()
    return list

def runCmd(cmd):
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    result, error = proc.communicate()

    if proc.wait() is None:
       print(error, file=sys.stderr)
       raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return result.replace("\n", "")

def getAttr(attrTab):
    attrPath = '/'.join(attrTab)
    try:
        attrFile = open(attrPath, 'r')
        attr = attrFile.read()
    except IOError:
        attr = None
    return attr.replace("\n", "")

def dumpPartitionParams(partition, label, fstype, size):
    print("Partition    ", partition, file=sys.stderr)
    print("Label:       ", label, file=sys.stderr)
    print("FileSystem:  ", fstype, file=sys.stderr)
    print("Size:         %s MB" %  size, file=sys.stderr)
    print("\n")

def verifyPartition(partition, reqLabel, reqFstype, reqSize):
    partPath = '/'.join(("/dev", partition))

    label = runCmd(("sudo", "blkid", "-s", "LABEL", "-o", "value", partPath))
    if label != reqLabel:
        raise InvalidPartitionError("Partition %s has invalid label %s. '%s' is required." % (partPath, label, reqLabel))

    fstype = runCmd(("sudo", "blkid", "-s", "TYPE", "-o", "value", partPath))
    if fstype != reqFstype:
        raise InvalidPartitionError("Partition %s has invalid fs type %s. '%s' is required." % (partition, fstype, reqFstype))

    sizeStr = getAttr(("/sys", "class", "block", partition, "size"))
    if sizeStr is not None:
        size = (string.atoi(sizeStr) * 512) / (1000*1000)
    else:
        size = 0
    if size < reqSize:
        raise InvalidPartitionError("Partition %s has invalid size %s (less than %s MB)." % (partition, size, reqSize))

    dumpPartitionParams(partition, label, fstype, size)

def checkPartitions(partitions):

    if len(partitions) < 3:
        raise InvalidPartitionError("There should be at least 3 partitions.")

    # First partition should be rootfs
    partition = partitions[0]
    verifyPartition(partition, "rootfs", "ext4", ROOTFS_SIZE)

    # Second partition should be config
    partition = partitions[1]
    verifyPartition(partition, "config", "ext4", CONFIG_SIZE)

    # Second partition should be kernel
    partition = partitions[2]
    verifyPartition(partition, "kernel", "vfat", KERNEL_SIZE)


def getConfirmation(true, false):
    while True:
        line = raw_input()
        if line == true:
            return True
        elif line == false:
            return False

def getProjectName(dirPath):
    dirList = dirPath.split('/')
    lastDir = dirList[len(dirList) - 1]
    if lastDir is '':
        lastDir = dirList[len(dirList) - 2]
    if re.match('sbuild-*', lastDir) is None:
        return None

    return lastDir.replace("sbuild-", "")

def findNfsRootPackage(project, path):
    list = []
    for file in os.listdir(path):
        if re.match(("%s-nfsroot-" % project), file):
            list.append(file)
    cnt = len(list)
    if cnt == 0:
        print("There's no nfsroot package at %s" % path)
        return None
    elif cnt > 1:
        print("There are more than one nfsroot package in %s" % path)
        for idx in range(cnt):
            print("[%d] %s:" % (idx, list[idx]))
        if getConfirmation("0", "1") is True:
            return '/'.join((path, list[0]))

    return '/'.join((path, list[0]))

def findKernelPackage(path):
    currPath = os.getcwd()
    kernelPath = str("%s/%starget/vmlinuz" % (currPath, path))
    if os.path.exists(kernelPath) is True:
        print(kernelPath)
        return kernelPath
    else:
        print("There is no vmlinuz package at path %s" % kernelPath)
    return None

def createTmpDir():
    tmpDir = "/tmp/testMount"
    if not os.path.exists(tmpDir):
        os.makedirs(tmpDir)
    return tmpDir

def mountTmpDir(tmpDir, device):
    print("Mounting %s on %s." % (device, tmpDir))
    runCmd(("sudo", "mount", device, tmpDir))
    print("%s mounted successfully." % device)

def unmountTmpDir(tmpDir):
    print("Unmounting %s." % tmpDir)
    runCmd(("sudo", "umount", tmpDir))
    print("%s unmounted." % tmpDir)

def clearTmpDir(tmpDir):
    shutil.rmtree(tmpDir)

def copyNfsRoot(device, package):
    try:
        tmpDir = createTmpDir()
        mountTmpDir(tmpDir, device)
        print("Extracting %s at %s. Please wait..." % (package, tmpDir))
        runCmd(("sudo", "tar", "-xzf", package, "-C", tmpDir))
        print("Extracking was finished successfully.")
    except SystemExit:
        unmountTmpDir(tmpDir)
        clearTmpDir(tmpDir)
        return

    unmountTmpDir(tmpDir)
    clearTmpDir(tmpDir)

def copyKernel(device, package):
    try:
        tmpDir = createTmpDir()
        mountTmpDir(tmpDir, device)
        print("Copying %s at %s. Please wait..." % (package, tmpDir))
        runCmd(("sudo", "cp", "-v", package, tmpDir))
        print("Copying was finished successfully.")
    except SystemExit:
        unmountTmpDir(tmpDir)
        clearTmpDir(tmpDir)
        return

    unmountTmpDir(tmpDir)
    clearTmpDir(tmpDir)
    
def clearDevice(device, cnt):
    for idx in range(cnt):
        runCmd(("sudo", "parted", device, "rm", str(idx + 1)))

def createAllPartitions(device):
    # all values are in 512B sectors
    runCmd(("sudo", "parted", device, "mktable", "msdos"))
    # create rootfs partition
    startSize = 2048
    endSize = 300
    print("%s" % device)
    runCmd(("sudo", "parted", "-a", "cyl", device, "mkpart", "primary", "0", str('%s' % endSize)))
    runCmd(("sudo", "mkfs.ext4", str('%s1' % device), "-L", "rootfs"))

    # create config partition
    startSize = endSize
    endSize += CONFIG_SIZE + 2
    runCmd(("sudo", "parted", str('%s' % device), "mkpart", "primary", str('%s' % startSize), str('%s' % endSize)))
    runCmd(("sudo", "mkfs.ext4", str('%s2' % device), "-L", "config"))

    # create kernel partition
    startSize = endSize
    endSize += KERNEL_SIZE + 2
    runCmd(("sudo", "parted", str('%s' % device), "mkpart", "primary", "fat32", str('%s' % startSize), str('%s' % endSize)))
    runCmd(("sudo", "mkfs.fat", "-F", "32", str('%s3' % device), "-n", "kernel"))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--nfsroot-path', '-p', action=verifyPathAction,
                        help='path to nfsroot package from sbuild-*')
    parser.add_argument('--device', '-d', action=readDeviceAction,
                        help='device name from /dev/* e.g. sdc')

    try:
        args = parser.parse_args()
    except ValueError as e:
        raise SystemExit("Parsing arguments failed.\n", e)
    except OSError as e:
        raise SystemExit(e)

    if args.device is None or args.nfsroot_path is None:
        print("Invalid parameters. There's no device defined. See help for more info. (--help|-h)");
        return

    print("\n")
    print("Device:    /dev/%s" % args.device, file=sys.stderr)
    vendor = getAttr(("/sys", "block", args.device, "device", "vendor"))
    print("Vendor:    %s" % vendor, file=sys.stderr)
    model = getAttr(("/sys", "block", args.device, "device", "model"))
    print("Model:     %s" % model, file=sys.stderr)
    sizeStr = getAttr(("/sys", "block", args.device, "size"))
    if sizeStr is not None:
        size = (string.atoi(sizeStr) * 512) / (1000*1000*1000)
    else:
        size = 0
    print("Size:      %s GB" % size, file=sys.stderr)
    print("\n")


    partList = findPartitions("/dev", args.device)
    needFormat = False
    if len(partList) == 0:
        print("There're no partitions at device: %s" % args.device)
        needFormat = True
    else:
        print("Found partitions:")
        try:
            checkPartitions(partList)
        except InvalidPartitionError as e:
            print("Parsing partition failed.\n", e)
            needFormat = True

    if needFormat is True:
        print("Partition table has invalid format. Would You like to format device and create correct one? [Yes/No]")
        if getConfirmation("Yes", "No") is True:
            device = os.path.join("/dev", args.device)
            clearDevice(device, len(partList))
            createAllPartitions(device)
            partList = findPartitions("/dev", args.device)
            checkPartitions(partList)

    project = getProjectName(args.nfsroot_path)

    print('Searching nfsroot in %s' % project)

    package = findNfsRootPackage(project, args.nfsroot_path)
    rootfs = os.path.join("/dev", partList[0])
    copyNfsRoot(rootfs, package)
    package = findKernelPackage(args.nfsroot_path)
    kernelPart = os.path.join("/dev", partList[2])
    copyKernel(kernelPart, package);


if __name__ == '__main__':
    main()
