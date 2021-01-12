#!/bin/sh

DRM_BIN_PART="$1"
DRM_BIN_TARGET="$2"

# separate variable, can be used for testing
DRM_BIN_SOURCE=/dev/"$DRM_BIN_PART"
#DRM_BIN_SOURCE="$DRM_BIN_PART"

if [ -z "$DRM_BIN_SOURCE" -o -z "$DRM_BIN_TARGET" ]; then
    echo "Usage: ${0} <drm bin partition name> <drm bin target path>"
    exit 1
fi

DRM_BIN_SOURCE_HEADER_LENGTH=2

#little endian
i=0
while [ $i -lt $DRM_BIN_SOURCE_HEADER_LENGTH ]; do
    DRM_BIN_HEADER_BYTES=$(xxd -u -ps -l 1 -s $i "$DRM_BIN_SOURCE")$DRM_BIN_HEADER_BYTES
    i=$(expr $i + 1)
done

#DRM_BIN_HEADER_BYTE_1=$(xxd -u -ps -l 1 "$DRM_BIN_SOURCE")
#DRM_BIN_HEADER_BYTE_2=$(xxd -u -ps -l 1 -s 1 "$DRM_BIN_SOURCE")
#DRM_BIN_LENGTH=$((0x$DRM_BIN_HEADER_BYTE_2$DRM_BIN_HEADER_BYTE_1))
DRM_BIN_LENGTH=$((0x$DRM_BIN_HEADER_BYTES))

if [ -z "$DRM_BIN_LENGTH" ]; then
    echo "Error: invalid drm bin header"
    exit 1
fi

DRM_BIN_SOURCE_LENGTH=$(wc -c < "$DRM_BIN_SOURCE")
DRM_BIN_SOURCE_PAYLOAD_LENGTH=$((DRM_BIN_SOURCE_LENGTH-DRM_BIN_SOURCE_HEADER_LENGTH))
if [ "$DRM_BIN_SOURCE_PAYLOAD_LENGTH" -lt "$DRM_BIN_LENGTH" ]; then
    echo "Error: invalid drm bin source payload length ["$DRM_BIN_SOURCE_PAYLOAD_LENGTH" bytes] - "$DRM_BIN_LENGTH" bytes expected"
    exit 1
fi

IS_PART_NAND=false
if [ -f /sys/class/mtd/"$DRM_BIN_PART"/type ]; then
    if [ "$(cat /sys/class/mtd/"$DRM_BIN_PART"/type)" = "nand" ]; then
        IS_PART_NAND=true
    fi
fi

RESULT=0
if $IS_PART_NAND ; then
    nandcat -l $((DRM_BIN_LENGTH+DRM_BIN_SOURCE_HEADER_LENGTH)) "$DRM_BIN_SOURCE" > /tmp/"$DRM_BIN_PART"_tmp
    dd if=/tmp/"$DRM_BIN_PART"_tmp of="$DRM_BIN_TARGET" bs=1 count="$DRM_BIN_LENGTH" skip="$DRM_BIN_SOURCE_HEADER_LENGTH"
    RESULT="$?"
    rm -f /tmp/"$DRM_BIN_PART"_tmp
else
    dd if="$DRM_BIN_SOURCE" of="$DRM_BIN_TARGET" bs=1 count="$DRM_BIN_LENGTH" skip="$DRM_BIN_SOURCE_HEADER_LENGTH"
    RESULT="$?"
fi

if [ "$RESULT" != 0 ]; then
    echo "Error : failed to install drm bin ["$RESULT"]"
fi

exit $RESULT
