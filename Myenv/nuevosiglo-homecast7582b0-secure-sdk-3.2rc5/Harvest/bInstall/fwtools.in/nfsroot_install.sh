#!/bin/bash

#
# Scripts unpacks nfsroot package to the install directory (it should be nfs-exported directory)
#

print_header()
{
	echo -e "\x1b[32m$*\x1b[39m"
}

if [ $# -lt 2 ]; then
	echo usage: "${0##*/} <install_dir> <nfsroot_package> [<base_debug_nfsroot_package>]"
	echo ""
	echo " <signed_image> - install dir where to unpack nfsroot package (it should be nfs-exported directory). "
	echo " <nfsroot_package> - path to nfsroot package containing application image to install"
	echo " <base_debug_nfsroot_package> - path to debug nfsroot package which is used as installer in installation proces,"
	echo "                                it is optional parameter - does not need to be given when <nfsroot_package>"
	echo "                                is a debug version; in that case <nfsroot_package> is also used as installer"   
	echo ""
	echo "Remarks: "
	echo " * scripts uses sudo!"
	echo " * current content of $INSTALL_DIR will be erased and filled with nfsroot package content"
	exit 1;
fi

INSTALL_DIR=$1
NFS_SRC=$2
NFS_BASE=$3
NFS_INSTALLER=${NFS_SRC}
if [ -n "$NFS_BASE" ]; then
	NFS_INSTALLER=${NFS_BASE}
fi

print_header "Please confirm to delete content of dir: ${INSTALL_DIR} as root."
print_header "press [d] to delete"
print_header "      [c] to cancel"
read -n 1 DECISION
if [ "$DECISION" != "d" ]; then
	print_header "Nfsroot package unpacking cancelled!"
	exit 1
fi

sudo rm -rf ${INSTALL_DIR}/*

print_header ""
print_header "Unpacking nfsroot installer ${NFS_INSTALLER} ..."
sudo tar zxf ${NFS_INSTALLER} -C ${INSTALL_DIR}

if [ -n "$NFS_BASE" ]; then
	print_header "Unpacking nfsroot with application image ${NFS_SRC} ..."
	NFS_SRC_PATH=`readlink -f $NFS_SRC`
	WORK_DIR=./_nfsroot
	mkdir -p ${WORK_DIR}
	rm -rf ${WORK_DIR}/*
	pushd ${WORK_DIR} >/dev/null
	tar -xzf ${NFS_SRC_PATH} firmware/
	chmod -R +w firmware
	popd >/dev/null

	
	sudo rm -rf ${INSTALL_DIR}/firmware
	sudo cp -rf ${WORK_DIR}/firmware ${INSTALL_DIR}
	rm -rf ${WORK_DIR}
fi

print_header "Done."
