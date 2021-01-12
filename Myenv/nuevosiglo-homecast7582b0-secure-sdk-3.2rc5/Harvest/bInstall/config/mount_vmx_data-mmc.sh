#!/bin/sh

find_device()
{
    PART_LABEL=vmx_data${1}
    ALL_DEVS=`ls /dev/mmcblk?p?`
    MMC_DEVICE=""
    for CUR_DEV in ${ALL_DEVS}; do
        CUR_PART=`echo ${CUR_DEV} | sed 's#.*/##'`
        if [ -d /sys/class/block/${CUR_PART} ]; then
            LABEL=`cat /sys/class/block/${CUR_PART}/volname`
            if [ "${LABEL}" = "${PART_LABEL}" ]; then
                MMC_DEVICE=${CUR_DEV}
            fi
        fi
    done
}

format_and_mount_vmx()
{
	echo "Creating new VMX data partition ${2} (${1})"
    mke2fs -t ext4 ${1}
    if [ ${?} -ne 0 ]; then
        "Cannot format VMX data prtition ${2} (${1})"
        exit 1
    fi
    mount -o sync,noexec -t ext4 ${MMC_DEVICE} /etc/vmx${2}
    if [ ${?} -ne 0 ]; then
        "Cannot mount VMX data prtition ${2} (${1})"
        exit 1
    fi
}

mount_vmx_part()
{
	IS_MOUNTED=`mount | grep "/etc/vmx${1}\b"`
	if [ -z "${IS_MOUNTED}" ]; then
		find_device ${1}
        if [ -z "${MMC_DEVICE}" ]; then
            echo "Cannot find VMX data partition ${1}!"
            exit -1
        fi
		mount -o sync,noexec -t ext4 ${MMC_DEVICE} /etc/vmx${1}
		if [ ${?} -ne 0 ]; then
			format_and_mount_vmx ${MMC_DEVICE} ${1}
		fi
	else
		echo "VMX data partition ${1} is already mounted"
	fi
}

umount_vmx_part()
{
	IS_MOUNTED=`mount | grep "/etc/vmx${1}\b"`
	if [ -z "${IS_MOUNTED}" ]; then
		echo "VMX data partition ${1} is not mounted"
	else
		umount /etc/vmx${1}
	fi
}

if [ "${1}" = "start" ]; then
	mount_vmx_part 0
	mount_vmx_part 1
	chown -R 1000:1000 /etc/vmx0
	chown -R 1000:1000 /etc/vmx1
fi

if [ "${1}" = "stop" ]; then
	umount_vmx_part 0
	umount_vmx_part 1
fi
