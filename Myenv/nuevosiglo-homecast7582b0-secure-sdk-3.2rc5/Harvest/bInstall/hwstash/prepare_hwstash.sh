#!/bin/bash

HWSTASH_FILES=${1}/hwstash.files
HWSTASH_KEY_PLAIN=${1}/hwstash.key.plain
HWSTASH_KEY_ENC=${1}/hwstash.key.enc
ROOTFS=${2}
HWSTASH_IV=${ROOTFS}/etc/hwstash/hwstash.iv
HWSTASH_PKG=$(mktemp)
HWSTASH_PKG_ENC=${ROOTFS}/etc/hwstash/hwstash.pkg.enc
WORKDIR=$(mktemp -d)

echo "Preparing hwstash package..."

chown 0:1000 ${WORKDIR}
chmod 750 ${WORKDIR}
for F in $(cat ${HWSTASH_FILES}); do
    if [ ! -f ${ROOTFS}/${F} ]; then
        echo "Cannot find ${ROOTFS}/${F} file! Ignoring"
        continue
    fi

    USER=$(stat -c "%u" ${ROOTFS}/${F})
    GROUP=$(stat -c "%g" ${ROOTFS}/${F})
    FILENAME=$(echo -n ${F} | md5sum | awk '{ print $1 }')
    cp -a ${ROOTFS}/${F} ${WORKDIR}/${FILENAME}

    rm ${ROOTFS}/${F}

    ln -s /tmp/hwstash/${FILENAME} ${ROOTFS}/${F}
    chown -h ${USER}:${GROUP} ${ROOTFS}/${F}
done

mksquashfs4 ${WORKDIR} ${HWSTASH_PKG} -noappend
rm -rf ${WORKDIR}

mkdir -p ${ROOTFS}/etc/hwstash
cp -a ${HWSTASH_KEY_ENC} ${ROOTFS}/etc/hwstash/
openssl rand 16 >${HWSTASH_IV}
openssl enc -aes-256-cbc -K $(xxd -ps ${HWSTASH_KEY_PLAIN}) -iv $(xxd -ps ${HWSTASH_IV}) -in ${HWSTASH_PKG} -out ${HWSTASH_PKG_ENC}

chown -R 0:1000 ${ROOTFS}/etc/hwstash/
chmod -R ug-w,o-rwx ${ROOTFS}/etc/hwstash/

cp -a hwstash ${ROOTFS}/etc/init.d/
rm ${HWSTASH_PKG}
