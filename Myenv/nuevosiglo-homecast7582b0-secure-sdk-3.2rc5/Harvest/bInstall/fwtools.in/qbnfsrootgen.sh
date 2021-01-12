#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $DIR/packagegen_util.sh

if [ -f $DIR/qbsystemvariables.sh ]; then
	. $DIR/qbsystemvariables.sh
fi

#
# Scripts repacks nfsroot package by replacing application image to be installed
#

if [ $# -ne 3 ]; then
	echo usage: "${0##*/} <signed_image> <old_nfsroot_package> <output_folder>"
	echo ""
	echo " <signed_image> - path to signed Latens image"
	echo " <old_nfsroot_package> - path to nfsroot package"
	echo " <output_folder> - output folder to write new generated nfsroot package"
	exit 1;
fi

IMAGE_PATH=$1
NFS_PATH=$2
NFS_TGZ=$(basename $NFS_PATH)
OUT_DIR=`readlink -f $3`

print_header "Creating nfsroot package with signed image ... "
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

OUT_NFS_TGZ=${NFS_TGZ/-${PACKAGE_VERSION}/-${IMAGE_VERSION}}
OUT_NFS_TGZ=${OUT_NFS_TGZ/.tgz/_signed.tgz}

SIGNED_IMAGE=${IMAGE_PATH}
if [ "$IMG_KIND" = "LATENS" ]; then
	# Adding QB header to signed image if it does not exist
	$DIR/qbheader.sh ${IMAGE_PATH} ${IMAGE_PATH}_
	SIGNED_IMAGE=${IMAGE_PATH}_
fi

TMP_DIR=_nfsroot
IMAGE_DIR=firmware/src

rm -rf $TMP_DIR
mkdir -p $TMP_DIR

mkdir -p $SRM_ROOT/_tmp/custom_mk/Harvest/bInstall
FAKED_ENV=$( mktemp -p $SRM_ROOT/_tmp/custom_mk/Harvest/bInstall )
fake_perm="fakeroot -i $FAKED_ENV -s $FAKED_ENV"

$fake_perm tar zxf $NFS_PATH -C $TMP_DIR
$fake_perm cp ${SIGNED_IMAGE} $TMP_DIR/$IMAGE_DIR/$IMAGE_NAME
cd $TMP_DIR
$fake_perm tar czf ${OUT_NFS_TGZ} *
cd -

rm $FAKED_ENV

install -m 664 $TMP_DIR/${OUT_NFS_TGZ} ${OUT_DIR}
rm -rf $TMP_DIR

if [ "$IMG_KIND" = "LATENS" ]; then
	rm ${IMAGE_PATH}_
fi

print_header "Creating nfsroot package done: $OUT_DIR/$OUT_NFS_TGZ"
