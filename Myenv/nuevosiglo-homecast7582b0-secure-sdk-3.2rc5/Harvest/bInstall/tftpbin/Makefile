
all: clean install

DEFAULT_KERNELOPTS = CROSS_COMPILE=$(CROSS_COMPILE) -j9
ifdef MAGNUM_PLATFORM
	DEFAULT_KERNELOPTS += PLATFORM=$(MAGNUM_PLATFORM)
endif
ifdef MAGNUM_BCHP_VER
	DEFAULT_KERNELOPTS += BCHP_VER=$(MAGNUM_BCHP_VER)
endif

KERNELOPTS?=$(DEFAULT_KERNELOPTS)

kernel.rootfs:
	rm -rf .tmp
	mkdir .tmp
	tar zxvf ../${NFSROOT_PACK} -C .tmp
	echo -e '#!/bin/sh\n\n/sbin/init' > .tmp/init
	mv .tmp kernel.rootfs

kernel.build:
	rm -rf .tmp
	cp -a $(KERNELDIR) .tmp
	sed -i .tmp/.config \
		-e "/\<CONFIG_BLK_DEV_RAM\>/d" \
		-e "/\<CONFIG_BLK_DEV_RAM_COUNT\>/d" \
		-e "/\<CONFIG_BLK_DEV_RAM_SIZE\>/d" \
		-e "/\<CONFIG_BLK_DEV_INITRD\>/d" \
		-e "/\<CONFIG_INITRAMFS_SOURCE\>/d" \
		-e "/\<CONFIG_INITRAMFS_ROOT_UID\>/d" \
		-e "/\<CONFIG_INITRAMFS_ROOT_GID\>/d" \
		-e "/\<CONFIG_CMDLINE\>/d"
	echo 'CONFIG_BLK_DEV_RAM=y' >> .tmp/.config
	echo 'CONFIG_BLK_DEV_RAM_COUNT=2' >> .tmp/.config
	echo 'CONFIG_BLK_DEV_RAM_SIZE=8192' >> .tmp/.config
	echo 'CONFIG_BLK_DEV_INITRD=y' >> .tmp/.config
	echo 'CONFIG_INITRAMFS_SOURCE="$(shell pwd)/kernel.rootfs"' >> .tmp/.config
	echo 'CONFIG_INITRAMFS_ROOT_UID=0' >> .tmp/.config 
	echo 'CONFIG_INITRAMFS_ROOT_GID=0' >> .tmp/.config
	echo 'CONFIG_CMDLINE="rw"' >> .tmp/.config
	mv .tmp kernel.build

compile: kernel.build kernel.rootfs
	yes "" | srm-local-run make -C kernel.build $(KERNELOPTS) oldconfig vmlinux
	gzip kernel.build/vmlinux

install: compile
	install -m 644 kernel.build/vmlinux.gz tftp.bin

clean:
	rm -rf .tmp
	rm -rf kernel.rootfs
	rm -rf kernel.build
	rm -f tftp.bin
