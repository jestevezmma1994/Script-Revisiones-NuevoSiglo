#!/bin/bash

#
# Scripts adds 256-bytes Cubiware header to Signed Application Image
# 

byte_swap()
{
	OUT=""
	CNT=$(echo -n $1 | wc -c); for i in `seq 1 2 $CNT`; do OUT+=`echo -n $1 | cut -c $(( CNT - i ))-$((CNT - i + 1))`; done
}

if [ $# -ne 2 ]; then
  echo usage: "${0##*/} <input_file> <output_file>"
  exit 1;
fi

if echo qbfw | cmp -n 4 - $1; then
	echo "It seems that '$1' already contains QB header (already signed?):"
	echo "press [s] to sign anyway"
	echo "      [c] to cancel"
	read -n 1 DECISION
	if [ "$DECISION" == "c" ]; then
		echo "Header already existed. Copying image without changes."
		cat $1 >> $2
		exit 0
	fi
fi

TOTAL_SIZE=$(stat -c%s "$1")

# QB header BEGIN
echo -n qbfw > $2
# Insert total size
byte_swap $(printf "%08x" $TOTAL_SIZE)
echo $OUT | xxd -r -p | dd conv=notrunc oflag=append of=$2 >/dev/null 2>&1
# Pad to 256 bytes
dd if=/dev/zero of=$2 bs=248 count=1 conv=notrunc oflag=append >/dev/null 2>&1
# QB header END

cat $1 >> $2

echo "QB header added."
