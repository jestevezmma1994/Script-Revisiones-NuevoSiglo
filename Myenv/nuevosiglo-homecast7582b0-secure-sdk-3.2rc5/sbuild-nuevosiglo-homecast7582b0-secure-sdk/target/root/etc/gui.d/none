#!/bin/sh

echo "0" > /sys/fp/animation

SANITYCHECKERDIR=/etc/vod/sanityChecker
SANITYRESURRECTIONFILE=${SANITYCHECKERDIR}/resurrectionCounter
RESURRECTIONTIMEOUT=180
RESURRECTIONTHRESHOLD=3

if [ ! -d ${SANITYCHECKERDIR} ]; then
    mkdir -p ${SANITYCHECKERDIR}
fi
if [ ! -f ${SANITYRESURRECTIONFILE} ]; then
    echo "1" > ${SANITYRESURRECTIONFILE}
    RESURRECTIONCOUNTER=1
else
    RESURRECTIONCOUNTER=$(cat ${SANITYRESURRECTIONFILE})
    RESURRECTIONCOUNTER=$((${RESURRECTIONCOUNTER} + 1))
    echo "${RESURRECTIONCOUNTER}" > ${SANITYRESURRECTIONFILE}
fi

if [ ${RESURRECTIONCOUNTER} -gt ${RESURRECTIONTHRESHOLD} ]; then
    rm -f ${SANITYRESURRECTIONFILE}
    touch /etc/vod/factoryReset
    echo "Resurrection ..." > /dev/fp
    /usr/local/bin/qbconf_ctl --set-null GUI
    while [ 1 ]; do
        /usr/local/bin/soft-reboot.sh
    done
fi

while [ 1 ]; do

    ip=$(ifconfig eth0 | grep 'inet addr' | cut -d: -f2 | cut -d\  -f1)
    if [ -z "$ip" ]; then
        echo "noip" > /dev/fp
        sleep 1
        continue
    fi

    echo "ip" > /dev/fp
    sleep 1

    for i in `seq 1 4`; do
        num=$( echo $ip | cut -d. -f$i)
        echo $num > /dev/fp
        sleep 2
    done

    if [ ${RESURRECTIONCOUNTER} -gt 0 ]; then
        UPTIMESEC=$(sed -r 's|^([0-9]+)\..*$|\1|' < /proc/uptime)
        if [ ${UPTIMESEC} -ge ${RESURRECTIONTIMEOUT} ]; then
            RESURRECTIONCOUNTER=0
            echo "${RESURRECTIONCOUNTER}" > ${SANITYRESURRECTIONFILE}
        fi
    fi

    sleep 3
done
