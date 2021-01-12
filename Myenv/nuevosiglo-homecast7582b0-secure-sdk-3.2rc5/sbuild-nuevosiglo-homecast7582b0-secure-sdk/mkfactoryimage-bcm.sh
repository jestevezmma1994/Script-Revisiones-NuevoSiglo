#!/bin/bash

# This script creates images for manufacturing write to flash
# Must be run in sbuild directory

print_header()
{
	echo -e "\x1b[32m$*\x1b[39m"
}

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

export NAND_PAGE=128KiB
export NAND_SUBPAGE=2048
export NAND_MINIO=2048
PREVDIR=`pwd`
if [ "$(which ubinize 2>/dev/null)" != "" ]; then
	UBINIZE_CMD="ubinize"
elif [ -f host/local/usr/bin/ubinize ]; then
	UBINIZE_CMD="../host/local/usr/bin/ubinize"
else
	echo "ubinize command not found rebuild OpenSource/mtd/mtd-host.comp or install mtd-utils on your host"
	exit 1
fi
export UBINIZE_CMD

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

QBIMAGE_KERNEL=`cat ../target/maps/all.maps | awk '/vmlinuz0/ {print $6}' | grep -o qbimage`
QBIMAGE_ROOTFS=`cat ../target/maps/all.maps | awk '/rootfs0/ {print $6}' | grep -o qbimage`
QBIMAGE_SPLASH=`cat ../target/maps/all.maps | awk '/logo/ {print $6}' | grep -o qbimage`
QBIMAGE_NVRAM=`cat ../target/maps/all.maps | awk '/nvram/ {print $6}' | grep -o qbimage`

cat ../target/maps/nand.map | awk '($6 ~ /fwimage/) && ($6 !~ /ubifs/) && ($6 !~ /squbifs/) {printf "echo NAND - %s - %s; cp fwtools/fwsrc/%s NAND_%s_%s 2>/dev/null\n",$1,$4,$4,$1,$4 >>"_script"}'
cat ../target/maps/nand.map | \
	awk '($6 ~ /fwimage/) && (($6 ~ /ubifs/) || ($6 ~ /squbifs/)) \
		{printf "[volume]\nmode=ubi\nvol_id=0\nvol_type=static\nvol_name=UBI_%s\nimage=fwtools/fwsrc/%s\n",$5,$4 >sprintf("%s_%s.ini",$1,$4); \
		printf "echo NAND - %s - rootfs.ubi; %s -o NAND_%s_%s -p %s -m %s -s %s %s_%s.ini\n",$1,ENVIRON["UBINIZE_CMD"],$1,$4,ENVIRON["NAND_PAGE"],ENVIRON["NAND_SUBPAGE"],ENVIRON["NAND_MINIO"],$1,$4 >>"_script"}'

IS_VMX=`cat ../target/maps/nand.map | awk '/vmlinuz0/ {print $4}' | grep vmx`
IS_LATENS=`cat ../target/maps/nand.map | awk '/vmlinuz0/ {print $4}' | grep latens`
IS_SEC3=`cat ../target/maps/nand.map | awk '/slot0/ {print $4}' | grep qb`
IS_VMX2=`cat ../target/maps/nand.map | awk '/slot0/ {print $4}' | grep vmx`

if [ -z "$IS_VMX" ] && [ -z "$IS_LATENS" ] && [ -z "$IS_VMX2" ] && [ -z "$IS_SEC3" ]; then
	ROOTFS_IMG=fwtools/fwsrc/`cat ../target/maps/nand.map | awk '/rootfs0/ {print $4}'`
	KERNEL_IMG=fwtools/fwsrc/`cat ../target/maps/nand.map | awk '/vmlinuz0/ {print $4}'`
fi
SPLASH_IMG=fwtools/fwsrc/`cat ../target/maps/all.maps | awk '/logo/ {print $4;exit}'`

if [ -z "$IS_LATENS" ]; then
	cat ../target/maps/spi.map | awk '($6 ~ /fwimage/) {printf "echo SPI - %s - %s; dd if=fwtools/fwsrc/%s of=SPI_0x00000000 bs=1024 seek=$(( %s / 1024 )) conv=notrunc 2>/dev/null\n",$1,$4,$4,$1,$1 >>"_script"}'
	cat ../target/maps/spi.map | awk '($6 ~ /fwimage/) {printf "cp fwtools/fwsrc/%s SPI_%s_%s 2>/dev/null\n",$4,$1,$4 >>"_script"}'
else
	cat ../target/maps/spi.map | awk '($6 ~ /fwimage/) {printf "echo SPI - %s - %s; cp fwtools/fwsrc/%s SPI_%s_%s 2>/dev/null\n",$1,$4,$4,$1,$4 >>"_script"}'
fi

print_header "Generating signatures ..."

print_signature()
{
	echo "$1: "`echo $2 | head -c 4`"..."`echo -n $2 | tail -c 4`
}

if [ -z "$IS_VMX" ] && [ -z "$IS_LATENS" ] && [ -z "$IS_VMX2" ] && [ -z "$IS_SEC3" ]; then
	if [ -z "$QBIMAGE_ROOTFS" ]; then
		ROOTFS_SIG=`jclient sign sha256 -t $ROOTFS_IMG with 52` || exit $?
		ROOTFS_SIG=`echo $ROOTFS_SIG | awk '{print toupper($1)};'` || exit $?
		print_signature "RootFS" "$ROOTFS_SIG"
	else
		echo "RootFS signature in QBBinaryImage"
	fi
	if [ -z "$QBIMAGE_KERNEL" ]; then
		KERNEL_SIG=`jclient sign sha256 -t $KERNEL_IMG with 52` || exit $?
		KERNEL_SIG=`echo $KERNEL_SIG | awk '{print toupper($1)};'` || exit $?
		print_signature "Kernel" "$KERNEL_SIG"
	else
		echo "Kernel signature in QBBinaryImage"
	fi
fi

if [ -z "$QBIMAGE_SPLASH" ]; then
	SPLASH_SIG=`jclient sign sha256 -t $SPLASH_IMG with 52` || exit $?
	SPLASH_SIG=`echo $SPLASH_SIG | awk '{print toupper($1)};'` || exit $?
	print_signature "Splash" "$SPLASH_SIG"
else
	echo "Splash signature in QBBinaryImage"
fi

print_header "Creating NVRAM ..."

if [ -z "$QBIMAGE_NVRAM" ]; then
	NVRAM_IMG=`cat ../target/maps/spi.map | awk '/nvram/ && !/nvramB/ {print "fwtools/fwsrc/" $4}'`
	if [ -z "$NVRAM_IMG" ]; then
		echo "NVRAM is on NAND"
		NVRAM_IMG=fwtools/fwsrc/`cat ../target/maps/nand.map | awk '/nvram/ && !/nvramB/ {print $4}'`
		NVRAMB_IMG=fwtools/fwsrc/`cat ../target/maps/nand.map | awk '/nvramB/ {print $4}'`
		NVRAM_SIZE=0x20000
	else
		echo "NVRAM is on SPI"
		NVRAMB_IMG=fwtools/fwsrc/`cat ../target/maps/spi.map | awk '/nvramB/ {print $4}'`
		NVRAM_SIZE=`cat ../target/maps/spi.map | awk '/nvram/ && !/nvramB/ {print $2}'`
	fi
else
	echo "NVRAM uses QBBinaryImage"
	NVRAM_IMG=fwtools/fwsrc/`cat ../target/maps/all.maps | awk '/nvram/ && !/nvramB/ {print $4}'`
	NVRAMB_IMG=fwtools/fwsrc/`cat ../target/maps/all.maps | awk '/nvramB/ {print $4}'`
	NVRAM_SIZE=$(echo '(32 * 1024)' - $(gcc -dM -E ../target/qbimage/QBBinaryImage.h | grep QB_IMAGE_HEADER_SIZE | sed "s/[^(]*\(.*\)/\1/") | bc)
fi

if [ -f ../target/root/etc/keys/nvram ] && [ -z "$QBIMAGE_NVRAM" ]; then
	cp ../target/root/etc/keys/nvram ./nvram_key
	echo "NVRAM encryption key present"
fi

echo "NVRAM size $NVRAM_SIZE"

write_nvram_var()
{
	echo "$3 -> $1"
	../host/local/usr/bin/nvimage $1 $NVRAM_SIZE $3 $4 || exit $?
	echo "$3 -> $2"
	../host/local/usr/bin/nvimage $2 $NVRAM_SIZE $3 $4 || exit $?
}

if [ -z "$IS_VMX" ] && [ -z "$IS_LATENS" ] && [ -z "$IS_VMX2" ] && [ -z "$IS_SEC3" ]; then
	if [ -z "$QBIMAGE_ROOTFS" ]; then
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG rootfs0_sign $ROOTFS_SIG
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG rootfs0_size `stat -c %s $ROOTFS_IMG`
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG rootfs1_sign $ROOTFS_SIG
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG rootfs1_size `stat -c %s $ROOTFS_IMG`
	fi
	if [ -z "$QBIMAGE_KERNEL" ]; then
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG vmlinuz0_sign $KERNEL_SIG
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG vmlinuz0_size `stat -c %s $KERNEL_IMG`
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG vmlinuz1_sign $KERNEL_SIG
		write_nvram_var $NVRAM_IMG $NVRAMB_IMG vmlinuz1_size `stat -c %s $KERNEL_IMG`
	fi
fi

if [ -z "$QBIMAGE_SPLASH" ]; then
	write_nvram_var $NVRAM_IMG $NVRAMB_IMG splash_sign $SPLASH_SIG
	write_nvram_var $NVRAM_IMG $NVRAMB_IMG splash_size `stat -c %s $SPLASH_IMG`
fi

write_nvram_var $NVRAM_IMG $NVRAMB_IMG SV_VERSION NEW
write_nvram_var $NVRAM_IMG $NVRAMB_IMG SV_STATUS NEW
write_nvram_var $NVRAM_IMG $NVRAMB_IMG SV_SLOT 0

if [ ! -z "$QBIMAGE_NVRAM" ]; then
	../host/local/usr/bin/mkqbbinaryimage.py --mode dynamic --encryption aes256-cbc --append-data nvram 219  $NVRAM_IMG ./nvram_a.qb
	../host/local/usr/bin/mkqbbinaryimage.py --mode dynamic --encryption aes256-cbc --append-data nvramB 219  $NVRAMB_IMG ./nvram_b.qb
	mv -f ./nvram_a.qb $NVRAM_IMG
	mv -f ./nvram_b.qb $NVRAMB_IMG
fi

if [ -f ../target/root/etc/keys/nvram ] && [ -z "$QBIMAGE_NVRAM" ]; then
	rm -f ./nvram_key
fi

print_header "Writing images ..."

/bin/bash _script
if [ -f "../$PKG_TGZ" ]; then
	rm ../$PKG_TGZ
fi

rm -rf fwtools _script *.ini polarssl_rsa_sign rsa_priv.txt

if [ -z "$PKG_TGZ" ]; then
	echo "Package name is empty"
	exit 1
fi

tar -czf ../$PKG_TGZ *

print_header "Done ..."
cd $PREVDIR
rm -rf _fwimage

