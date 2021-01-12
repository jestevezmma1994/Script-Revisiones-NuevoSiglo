#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $DIR/packagegen_util.sh

if [ -f $DIR/qbsystemvariables.sh ]; then
	. $DIR/qbsystemvariables.sh
fi

#
# Script creates upgrade package containing given Signed Application Image

if [ $# -lt 3 ]; then
	echo usage: "${0##*/} <signed_image> <nfsroot_path> <output_folder> [<upgrade_key_file>]"
	echo ""
	echo " <signed_image> - path to signed Latens image"
	echo " <nfsroot_path> - path to nfsroot package"
	echo " <output_folder> - output folder to write generated package"
	echo " <upgrade_key_file> - optional key for use to sign package (without use of keyserver). If key is not given, package is signed using keyserver"
	exit 1;
fi

IMAGE_PATH=$1
NFS_PATH=$2
NFS_TGZ=$(basename $NFS_PATH)
TOOLS_DIR=$(dirname $NFS_PATH)
OUT_DIR=`readlink -f $3`
S_RUN_OFFLINE=no
if [ -n "$4" ]; then
	S_RUN_OFFLINE=yes
	UPGRADE_KEY=`readlink -f $4`
fi
export S_RUN_OFFLINE

print_header "Creating upgrade package with signed image ... "
print_info "Using base nfsroot package: $NFS_PATH"
print_info "Using signed image: $IMAGE_PATH"

if [ ! -f $IMAGE_PATH ]; then
        echo "File $IMAGE_PATH not found!"
        exit 1
fi
if [ ! -f $NFS_PATH ]; then
        echo "File $NFS_PATH not found!"
        exit 1
fi
if [ -z `echo $NFS_TGZ | grep nfsroot` ]; then
        echo "Incorrect nfsroot package given!"
        exit 1
fi
if [ ! -d $OUT_DIR ]; then
        echo "Output directory $OUT_DIR not found!"
        exit 1
fi

if [ "yes" = "${S_INSTALL_VMX_UPGRADE}" ]; then
	IMG_KIND="VMX"
    if [ "yes" = "${S_INSTALL_VMX_UPGRADE_2}" ]; then
	    IMAGE_NAME=BOOTIMAGE.vmx
    else
    	IMAGE_NAME=ROOTFS.vmx
    fi
elif [ "yes" = "${S_INSTALL_LATENS_UPGRADE}" ]; then
	IMG_KIND="LATENS"
        if [ "yes" = "${S_INSTALL_LATENS_UPGRADE_2}" ]; then
            IMAGE_NAME=BOOTIMAGE.latens
        else
            IMAGE_NAME=ROOTFS.latens
        fi
fi

print_info "Image kind: $IMG_KIND"

if [ "$IMG_KIND" = "LATENS" ]; then
	check_latens_package_version $NFS_TGZ $IMAGE_PATH
elif [ "$IMG_KIND" = "VMX" ]; then
	PACKAGE_VERSION="$(echo $NFS_TGZ | awk -F'nfsroot-' '{print $NF}' | sed 's/^\(.*\).tgz$/\1/')"
	IMAGE_VERSION=${PACKAGE_VERSION}
else
	print_error "Unsupported image kind"
	exit 1
fi

SIGNED_DIR=firmware/_signedsrc
ORGINAL_DIR=firmware/src

UPG_TGZ=${NFS_TGZ/-nfsroot/-upgrade}
UPG_TGZ=${UPG_TGZ/-${PACKAGE_VERSION}/-${IMAGE_VERSION}}
UPG_TGZ=${UPG_TGZ/.tgz/_signed.tgz}
WORK_DIR=./.nfsroot
mkdir -p $WORK_DIR
rm -rf $WORK_DIR/*

SIGNED_IMAGE=${IMAGE_PATH}
if [ "$IMG_KIND" = "LATENS" ]; then
	# Adding QB header to signed image if it does not exist
	$DIR/qbheader.sh ${IMAGE_PATH} ${IMAGE_PATH}_
	SIGNED_IMAGE=${IMAGE_PATH}_
fi

NFSROOT_PACKAGE=`readlink -f $NFS_PATH`
pushd .
cd $WORK_DIR
tar -xzf $NFSROOT_PACKAGE firmware/
chmod -R +w firmware
popd

mkdir -p $WORK_DIR/$SIGNED_DIR
cp -rf $WORK_DIR/$ORGINAL_DIR/* $WORK_DIR/$SIGNED_DIR/
cp ${SIGNED_IMAGE} $WORK_DIR/$SIGNED_DIR/$IMAGE_NAME

if [ "$S_RUN_OFFLINE" = "yes" ]; then
	export S_INSTALL_CREATE_RSA_UPGRADE=yes

	SPLASH_SIGN=cp7584a1-img-smp.sig
	cp ${TOOLS_DIR}/${SPLASH_SIGN} $WORK_DIR/$SIGNED_DIR/
else
	export S_INSTALL_KEYSERVER_RSA_UPGRADE_OUTER_KEY_ID=$S_INSTALL_KEYSERVER_RSA_UPGRADE_KEY_ID
fi

MKUPGRADE_PATH=$DIR/mkupgrade.sh
cd $WORK_DIR

if [ "$IMAGE_VERSION" = "CURRENT" ]; then
	RELEASE=$IMAGE_VERSION
else
	RELEASE=`echo $IMAGE_VERSION | cut -d'r' -f1`
fi

if [ "$S_RUN_OFFLINE" = "yes" ]; then
	$MKUPGRADE_PATH $SIGNED_DIR $OUT_DIR/$UPG_TGZ $RELEASE $UPGRADE_KEY
else
	$MKUPGRADE_PATH $SIGNED_DIR $OUT_DIR/$UPG_TGZ $RELEASE
fi

cd -
rm -rf $WORK_DIR

if [ "$IMG_KIND" = "LATENS" ]; then
	rm ${IMAGE_PATH}_
fi

print_header "Creating upgrade package done: $OUT_DIR/$UPG_TGZ"

