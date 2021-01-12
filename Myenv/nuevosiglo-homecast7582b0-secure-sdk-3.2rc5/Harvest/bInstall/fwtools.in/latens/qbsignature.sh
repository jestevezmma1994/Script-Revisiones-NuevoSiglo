#!/bin/bash

#
# Script signs given Plain Application Image with QB Fake Signature.
# It is required to run application on Development Bootloader
#

QB_SIGNATURE_SIZE=16

byte_swap()
{
	OUT=""
	CNT=$(echo -n $1 | wc -c); for i in `seq 1 2 $CNT`; do OUT+=`echo -n $1 | cut -c $(( CNT - i ))-$((CNT - i + 1))`; done
}

if [ $# -ne 2 ]; then
	echo usage: "${0##*/} <input_file> <output_file>"
	exit 1;
fi

TOTAL_SIZE=$(($(stat -c%s "$1") + $QB_SIGNATURE_SIZE))

# QB test signature header BEGIN
echo -n abcd > $2
# Insert total size
byte_swap $(printf "%08x" $TOTAL_SIZE)
echo $OUT | xxd -r -p | dd conv=notrunc oflag=append of=$2 >/dev/null 2>&1
# Pad to 16 bytes
dd if=/dev/zero of=$2 bs=8 count=1 conv=notrunc oflag=append >/dev/null 2>&1
# QB test signature header END

cat $1 >> $2

echo "Fake signature created: $2"
