#!/bin/sh

# UBIFS needs at least 17 blocks, UBI volume reserves another 4 for internal purposes, we leave another 2 just in case
# Rest is reserved for bad block handling (-r option to ubiattach)
UBI_NEEDED_BLOCKS=23

if [ ! -e /dev/shm/ubi_ctrl ]; then
	UBICTRL_MAJOR=`cat /sys/class/misc/ubi_ctrl/dev | cut -d: -f1`
	UBICTRL_MINOR=`cat /sys/class/misc/ubi_ctrl/dev | cut -d: -f2`
	rm -f /dev/shm/ubi_ctrl
	mknod /dev/shm/ubi_ctrl c $UBICTRL_MAJOR $UBICTRL_MINOR
fi

ubi_attach() {
	ubidetach /dev/shm/ubi_ctrl -d $1 >/dev/null 2>&1
	ubiattach /dev/shm/ubi_ctrl -p /dev/mtd$1 -d $1 -r $2 >/dev/null 2>&1 || return 1
	rm -f /dev/shm/ubi$1
	mknod /dev/shm/ubi$1 c `cat /sys/class/ubi/ubi$1/dev | cut -d: -f1` `cat /sys/class/ubi/ubi$1/dev | cut -d: -f2`
}

format_vmx()
{
	#reset partition
	echo "Creating new VMX data $2 partition"
	ubidetach /dev/shm/ubi_ctrl -d $1 >/dev/null 2>&1
	flash_erase /dev/mtd$1 0 0
	ubiformat -y /dev/mtd$1 || return 1
	ubi_attach $1 $UBI_NEEDED_BLOCKS || return 1
	UBI_FREE=`cat /sys/class/ubi/ubi$1/avail_eraseblocks`
	ubimkvol /dev/shm/ubi$1 -N UBI_vmx_data$2 -S $UBI_FREE || return 1
	mount -o sync,noexec -t ubifs ubi$1:UBI_vmx_data$2 /etc/vmx$2
}

mount_vmx_part()
{
	IS_MOUNTED=`mount | grep "UBI_vmx_data$1\b"`
	if [ -z "$IS_MOUNTED" ]; then
		VMX_NUM=`grep "\<vmx_data$1\>" /proc/mtd | sed "s/mtd\([[:digit:]]*\): .*/\1/"`
		ubi_attach $VMX_NUM $UBI_NEEDED_BLOCKS
		mount -o sync,noexec -t ubifs ubi$VMX_NUM:UBI_vmx_data$1 /etc/vmx$1
		if [ $? -ne 0 ]; then
			format_vmx $VMX_NUM $1
		fi
	else
		echo "VMX data partition $1 is already mounted"
	fi
}

umount_vmx_part()
{
	IS_MOUNTED=`mount | grep "UBI_vmx_data$1\b"`
	if [ -z "$IS_MOUNTED" ]; then
		echo "VMX data partition $1 is not mounted"
	else
		VMX_NUM=`grep "\<vmx_data$1\>" /proc/mtd | sed "s/mtd\([[:digit:]]*\): .*/\1/"`
		umount /etc/vmx$1
		ubidetach /dev/shm/ubi_ctrl -d $VMX_NUM >/dev/null 2>&1
	fi
}

if [ "$1" = "start" ]; then 
	mount_vmx_part 0
	mount_vmx_part 1
	chown -R 1000:1000 /etc/vmx0
	chown -R 1000:1000 /etc/vmx1
fi

if [ "$1" = "stop" ]; then
	umount_vmx_part 0
	umount_vmx_part 1
fi
