#!/bin/bash
mkdir -p tmp_ubiroot
mksquashfs4 $1 tmp_rootfs.img
if [ "$S_KERNEL_USE_KEYCHAIN" == "yes" ]; then
    openssl rand -out ./tmp_rootfskey 32
    if [ "$S_INSTALL_USE_KEYSERVER" == "yes" ]; then
        echo "Using keyserver for ROOT FS KEY encryption"
        srm-host-run jclient encrypt -t tmp_rootfskey with $S_INSTALL_KEYSERVER_SYM_ROOTFS_KEY_ID > tmp_ubiroot/enckey.bin || exit $?
    else
        echo "Rootfs key at $S_BOOTKEYS_AES_ROOTFS"
        openssl aes-256-ecb -nopad -nosalt -in tmp_rootfskey -out tmp_ubiroot/enckey.bin -e -K $(cat $S_BOOTKEYS_AES_ROOTFS) -iv 00 -p || exit $?
    fi
    echo "Encrypting ROOT FS with one-time key"
    openssl aes-256-ecb -nopad -nosalt -in tmp_rootfs.img -out tmp_ubiroot/rootfs.img -e -K $(xxd -p -c 256 ./tmp_rootfskey) -iv 00 -p || exit $?
else
    if [ "$S_INSTALL_USE_KEYSERVER" == "yes" ]; then
        echo "Using keyserver for UBI FS encryption"
        srm-host-run jclient encrypt -t tmp_rootfs.img with $S_INSTALL_KEYSERVER_SYM_ROOTFS_KEY_ID > tmp_ubiroot/rootfs.img || exit $?
    else
        echo "Rootfs key at $S_BOOTKEYS_AES_ROOTFS"
        openssl aes-256-ecb -nopad -nosalt -in tmp_rootfs.img -out tmp_ubiroot/rootfs.img -e -K $(cat $S_BOOTKEYS_AES_ROOTFS) -iv 00 -p || exit $?
    fi
fi
mkfs.ubifs -d tmp_ubiroot -o $2 -m $3 -e $4 -c $5 -x none
rm -rf tmp_ubiroot tmp_rootfs.img tmp_rootfskey
