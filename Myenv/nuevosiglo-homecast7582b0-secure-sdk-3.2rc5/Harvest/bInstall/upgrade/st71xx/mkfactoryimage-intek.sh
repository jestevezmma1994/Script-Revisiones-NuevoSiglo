#!/bin/bash

# This script creates images for manufacturing write to flash
# Must be run in sbuild directory

print_header()
{
	echo -e "\x1b[32m$*\x1b[39m"
}

print_usage()
{
	print_info "Usage: ${0##*/} [OPTIONS]"
	print_info "Create factory image\n"
	print_info "  --no-u-boot       Do not include u-boot in image"
	exit 1
}

while [ ! -z "$1" ]; do
		case "$1" in
		--no-u-boot)	SKIP_UBOOT="yes"
						;;
		*)				print_usage
						;;
		esac
		shift
done

print_header "Checking package ..."
SRC_TGZ=`ls *.tgz | grep fwtools`
if [ -z "$SRC_TGZ" ]; then
	echo "No fwsrc package found, trying nfsroot"
	SRC_TGZ=`ls *.tgz | grep nfsroot`
	TGZ_TYPE="nfs"
	if [ -z "$SRC_TGZ" ]; then
		echo "No nfsroot package found, are you running from sbuild directory?"
		exit 1
	fi
else
	TGZ_TYPE="fw"
fi

if [ `echo $SRC_TGZ | wc -w` -ne 1 ]; then
	echo "Multiple packages found, select correct one:"
	SEL_NUM=0
	for SELECTION in $SRC_TGZ; do
		SEL_NUM=$(($SEL_NUM + 1))
		echo $SEL_NUM") "$SELECTION
	done
	read -n 1 FINAL_NUM
	SEL_NUM=0
	for SELECTION in $SRC_TGZ; do
		SEL_NUM=$(($SEL_NUM + 1))
		if [ $SEL_NUM -eq $FINAL_NUM ]; then
			SEL_TGZ=$SELECTION
			break
		fi
	done
else
	SEL_TGZ=$SRC_TGZ
fi

if [ -z "$SEL_TGZ" ]; then
	echo ""
	echo "Invalid selection"
	exit 1
fi
echo Using package $SEL_TGZ

if [ "$SKIP_UBOOT" != "yes" ]; then
	print_header "Checking bootloader ..."

	if [ -f ./target/boot/u-boot.bin ]; then
		echo "Bootloader found"
	else
		echo "Unable to find bootloader please pull and make Platforms/STx/u-boot-bin repository"
		exit 1
	fi
fi

PREVDIR=`pwd`
mkdir _fwimage 2>/dev/null
cd _fwimage
rm -rf *

print_header "Unpacking package ..."
if [ "$TGZ_TYPE" == "fw" ]; then
	tar -xzf ../$SEL_TGZ
	PKG_TGZ=$(echo $SEL_TGZ | sed -e "s/fwtools/mpimage/")
else
	tar -xzf ../$SEL_TGZ firmware/
	chmod -R +w firmware
	mv firmware fwtools
	mv fwtools/src fwtools/fwsrc
	PKG_TGZ=$(echo $SEL_TGZ | sed -e "s/nfsroot/mpimage/")
fi

print_header "Generating scripts ..."
if [ "$SKIP_UBOOT" == "yes" ]; then
	cat ../target/maps/nor.map | sort | \
		awk '($6 ~ /fwimage/) {printf "echo %s - %s ; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=fwtools/fwsrc/%s 2>/dev/null\n",$1,$4,strtonum($1) / 131072,strtonum($2) / 131072,$4} \
			/nvram/ {printf "echo %s - nvram; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=nvram.bin 2>/dev/null\n",$1,strtonum($1) / 131072,strtonum($2) / 131072} \
			!($6 ~ /fwimage/) && !/nvram/ && !/^#/ {printf "echo %s - empty; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=/dev/zero 2>/dev/null\n",$1, strtonum($1) / 131072,strtonum($2) / 131072}' >>_script
else
	cat ../target/maps/nor.map | sort | \
		awk '/u-boot/ {printf "echo %s - u-boot ; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=../target/boot/u-boot.bin 2>/dev/null\n",$1,strtonum($1) / 131072,strtonum($2) / 131072,$4} \
			($6 ~ /fwimage/) {printf "echo %s - %s ; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=fwtools/fwsrc/%s 2>/dev/null\n",$1,$4,strtonum($1) / 131072,strtonum($2) / 131072,$4} \
			/nvram/ {printf "echo %s - nvram; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=nvram.bin 2>/dev/null\n",$1,strtonum($1) / 131072,strtonum($2) / 131072} \
			!($6 ~ /fwimage/) && !/nvram/ && !/^#/ && !/u-boot/ {printf "echo %s - empty; dd status=noxfer conv=notrunc seek=%d count=%d bs=131072 of=NOR_IMAGE.bin if=/dev/zero 2>/dev/null\n",$1, strtonum($1) / 131072,strtonum($2) / 131072}' >>_script
fi

ROOTFS_IMG=fwtools/fwsrc/`cat ../target/maps/nor.map | awk '/rootfs0/ {print $4}'`
KERNEL_IMG=fwtools/fwsrc/`cat ../target/maps/nor.map | awk '/vmlinuz0/ {print $4}'`

print_header "Generating signatures ..."

print_signature()
{
	echo "$1: "`echo $2 | head -c 4`"..."`echo -n $2 | tail -c 4`
}

ROOTFS_SIG=`jclient sign S_INSTALL_KEYSERVER_RSA_BOOT_HASH -t $ROOTFS_IMG with S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID` || exit $?
ROOTFS_SIG=`echo $ROOTFS_SIG | awk '{print toupper($1)};'` || exit $?
print_signature "RootFS" "$ROOTFS_SIG"
KERNEL_SIG=`jclient sign S_INSTALL_KEYSERVER_RSA_BOOT_HASH -t $KERNEL_IMG with S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID` || exit $?
KERNEL_SIG=`echo $KERNEL_SIG | awk '{print toupper($1)};'` || exit $?
print_signature "Kernel" "$KERNEL_SIG"

print_header "Creating NVRAM ..."

NVRAM_IMG=fwtools/fwsrc/`cat ../target/maps/nor.map | awk '/nvram/ && !/red.nvram/ {print $4}'`
NVRAMB_IMG=fwtools/fwsrc/`cat ../target/maps/nor.map | awk '/nvram/ {print $4}'`

write_nvram_var()
{
	echo "$1 -> nvram.bin"
	../host/local/usr/bin/nvimage $1=$2 >/dev/null 2>/dev/null || exit $?
}

write_nvram_var rootfs0_sign $ROOTFS_SIG
write_nvram_var vmlinuz0_sign $KERNEL_SIG
write_nvram_var vmlinuz0_size `stat -c %s $KERNEL_IMG`
write_nvram_var rootfs1_sign $ROOTFS_SIG
write_nvram_var vmlinuz1_sign $KERNEL_SIG
write_nvram_var vmlinuz1_size `stat -c %s $KERNEL_IMG`
write_nvram_var qb_slot 0
write_nvram_var qb_status FLASHED
write_nvram_var qb_version FLASHED

print_header "Writing images ..."

/bin/bash _script
if [ -f "../$PKG_TGZ" ]; then
	rm ../$PKG_TGZ
fi

rm -rf fwtools _script nvram.bin

if [ -z "$PKG_TGZ" ]; then
	echo "Package name is empty"
	exit 1
fi

tar -czf ../$PKG_TGZ *

print_header "Done ..."
cd $PREVDIR
rm -rf _fwimage
