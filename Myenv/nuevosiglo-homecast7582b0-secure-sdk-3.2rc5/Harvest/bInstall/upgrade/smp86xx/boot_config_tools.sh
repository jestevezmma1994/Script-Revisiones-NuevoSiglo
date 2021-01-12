#!/bin/sh

[ -z "$CS_ID" ] && CS_ID=2
[ -z "$CS_PART" ] && CS_PART=2

X1PART=`cat /proc/mtd | grep "CS$CS_ID-Part"$(( $CS_PART + 0 )) | cut -d':' -f1 | cut -c4-`
X2PART=`cat /proc/mtd | grep "CS$CS_ID-Part"$(( $CS_PART + 1 )) | cut -d':' -f1 | cut -c4-`

get_boot_config () {
for i in $X1PART $X2PART; do
	if [ `grep "#" -c /dev/mtd$i` -ne 2 ]; then
		continue
	fi
	if [ ! -z "$1" ]; then  
		dd if=/dev/mtd$i bs=1k count=1 2>/dev/null | cut -d'#' -f2 | grep "^u." | sed "s/[^[:print:]]//g" | grep "^$1=" | cut -d'=' -f2
	else
		dd if=/dev/mtd$i bs=1k count=1 2>/dev/null | cut -d'#' -f2 | grep "^u." | sed "s/[^[:print:]]//g"
	fi
#	cat /dev/mtd$i | cut -d'#' -f2 | grep "^u." | sed "s/[^[:print:]]//g" | sed "s/\"\(.*\)\"/\1/" #do not remove quotes in string variables
	return 0;
done
return 1;
}

set_boot_config () {
CONFIG=`get_boot_config`
while [ ! -z "$1" ]; do
	CONFIG=`echo "$CONFIG" | grep -v "^$1"`
	CONFIG=`echo -e "$CONFIG\n$1=$2"`
	shift
	shift
done

if [ `dd if=/dev/mtd$X1PART bs=1k count=1 2>/dev/null | grep "#" -c` -ne 2 ]; then
	x1=$X2PART
	x2=$X1PART
else
	x1=$X1PART
	x2=$X2PART
fi

echo -e "#\n$CONFIG\n#\n" > /tmp/boot_config.tmp
for i in $x2 $x1; do
	flashcp /tmp/boot_config.tmp /dev/mtd$i
done
rm -f /tmp/boot_config.tmp
}
