###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
#       use variables described below for that purposes       #
###############################################################

#Variables:
# S_INSTALL_LANG - language suffix, see files in ./strings directory
# S_INSTALL_DO_STRIP - set it to 'yes' if you want you libraries to be stripped
# S_INSTALL_RCFILE - rcS file name. it is recommended to use the default one and to extend if it is needed. create new one only for specific firmware. note, the rcS.d links will be created only to existing init.d file, so if you don't want how to avoid some rcS entry just exclude init.d script.
# S_INSTALL_DEVTGZ - device tar ball file. use the default one and extend if you need more devices. do not create any other device tar ball unless it is absolutely needed to limit device nodes number.
# S_INSTALL_UPGRADE - set to "yes" if you want to produce upgrade package
# S_INSTALL_UPGRADE_ARCH - set to upgrade package architecture (default is SRM_BOARD)
# S_INSTALL_UPGRADE_IMG - set to upgrade package image prefix (default and recommended is "img")
# S_INSTALL_UPGRADE_PROJECT - set to project name in upgrade package (default and recommended is "smp")
# S_INSTALL_UPGRADE_PLATFORM_SPEC - define where to find platform specific upgrade scripts components
# S_INSTALL_NFSROOT - set to 'no' if you do not want nfsroot
# S_INSTALL_SKINTOOLS - set to 'no' if you do not want to create skinning package
# S_INSTALL_FTFPBIN - set to 'no' if tftp binary for integrated boot is not needed
# S_INSTALL_VMROOTCERT - set to yes if you want to install Verimatrix rootcert.pem
# S_INSTALL_REMOVE_RESTRICTED - list (space separated) of directories/files to be removed before building, e.g. = ${SRM_ROOT}/base ${MAGNUM_DIR}
#
# Variable for keys and certificates
# the default value for all *_DIR variable is "generic", please refer the generic dir for structure and key/cert generation instructions
# S_INSTALL_SSHKEYS_DIR - define dir of ssh keys set in ./keys dir, default is "generic"
# S_INSTALL_UPCCERT_DIR - UPgrade Connection CERTificates, define dir of sll connection with upgrade server in ./keys dir
# obsolite S_INSTALL_SSLCERT_DIR - the old name of S_INSTALL_UPCCERT_DIR, keeped for back compatibility
# S_INSTALL_UPPCERT_DIR - UPgrade Package CERTificates, define dir of upgrade package supplier certificate dir in ./keys dir

.EXPORT_ALL_VARIABLES:

.SILENT:

#default configuration
SHELL=/bin/bash

#harvest components
S_INSTALL_UPGRADE?=yes
S_INSTALL_NFSROOT?=yes
S_INSTALL_SKINTOOLS?=yes
S_INSTALL_PACKAGE?=no
S_INSTALL_TFTPBIN?=no
S_INSTALL_DRIVER?=yes

#settings
S_INSTALL_DO_STRIP?=yes
S_INSTALL_LANG?=en
S_INSTALL_VMROOTCERT?=no

#keys and certificates
S_INSTALL_SSHKEYS_DIR:=$(S_INSTALL_BSPEC)/keys/ssh
S_INSTALL_UPCCERT_DIR:=$(S_INSTALL_BSPEC)/keys/upgrade-https
S_INSTALL_UPPCERT_DIR:=$(S_INSTALL_BSPEC)/keys/upgrade-package

# Create old ECDSA-style upgrade files
S_INSTALL_LEGACY_ECDSA_UPGRADE?=no

# Create new RSA-style upgrade files
S_INSTALL_CREATE_RSA_UPGRADE?=no

# Use (in run-time) RSA-style upgrade
S_INSTALL_USE_RSA_UPGRADE?=no

# Use Key Server by default?
S_INSTALL_USE_KEYSERVER?=no
# RSA Keys
S_INSTALL_KEYSERVER_RSA_UPGRADE_KEY_ID?=1125
S_INSTALL_KEYSERVER_RSA_UPGRADE_INNER_KEY_ID?=$(S_INSTALL_KEYSERVER_RSA_UPGRADE_KEY_ID)
S_INSTALL_KEYSERVER_RSA_UPGRADE_OUTER_KEY_ID?=$(S_INSTALL_KEYSERVER_RSA_UPGRADE_KEY_ID)
S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID?=1126
S_INSTALL_KEYSERVER_RSA_SKIN_KEY_ID?=5
S_INSTALL_KEYSERVER_RSA_SKIN_HASH?=sha1
# Hashing function for boot signatures
S_INSTALL_KEYSERVER_RSA_BOOT_HASH?=sha1
# Hashing function for upgrade
S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH?=sha1
# Symmetric / AES / DES Keys
S_INSTALL_KEYSERVER_SYM_ROOTFS_KEY_ID?=1130
S_INSTALL_KEYSERVER_SYM_ROOTFS_MODE?=ecb
S_INSTALL_KEYSERVER_SYM_KERNEL_KEY_ID?=1127

# If using pre-encrypted kernel this is set to yes by the project
S_KERNEL_USE_KEYCHAIN?=no

# Using Verimatrix upgrade with reencryption
S_INSTALL_VMX_UPGRADE?=no

# Using Latens upgrade
S_INSTALL_LATENS_UPGRADE?=no

# Which driver variant to install, either FREE or PROP
# For compatibility reasons we will skip this step if target/drivers folder not found
S_INSTALL_DRIVER_VARIANT?=FREE

# Whether to use Cerberus checks, default=yes
S_INSTALL_USE_CERBERUS?=yes

ifeq (,$(S_MAP_DIR))
$(warning map.comp component is missing in the build!);
endif

# Allow making fake releases.
# The main purpose of this code is to allow creating automatically 99.99 releases required by testers team.
# To set fake version use shell environment QB_FAKE_RELEASE, for example:
# QB_FAKE_RELEASE=99.99
ifdef QB_FAKE_RELEASE
    ifneq (environment,$(origin QB_FAKE_RELEASE))
        $(error Fake release can be only set by environment variable)
    endif
    ifeq (,$(shell echo $(QB_FAKE_RELEASE) | grep -Eo '^[1-9][0-9]{0,1}\.(0|[1-9][0-9]{0,1})$$'))
        $(error Invalid fake release)
    endif
    SRM_RELEASE=$(QB_FAKE_RELEASE)
endif

#BOARD=$(SRM_BOARD)
DESTDIR=$(SRM_DESTDIR)
RELEASE=$(SRM_RELEASE)
ifneq (,$(SRM_RELEASE_CANDIDATE))
	VERSION=$(SRM_RELEASE)rc$(SRM_RELEASE_CANDIDATE)
else
	VERSION=$(SRM_RELEASE)
endif

UPGRADE = $(SRM_PROJECT)-upgrade
NFSROOT = $(SRM_PROJECT)-nfsroot
SKINTOOLS = $(SRM_PROJECT)-skintools
PACKAGE = $(SRM_PROJECT)-package
TFTPBIN = $(SRM_PROJECT)-tftpbin
DRIVER = $(SRM_PROJECT)-driver

BUILDDIR = $(SRM_ROOT)/_tmp/custom_mk/Harvest/bInstall
TOOLSDIR = $(BUILDDIR)/tools

MAP=$(S_MAP_DIR)/all.maps
MAPDIR=$(S_MAP_DIR)

HERE=$(shell pwd)

ifeq ($(shell mapreader.pl $(MAP) dst squbifs),)
    ifeq ($(shell mapreader.pl $(MAP) attr qbimage),)
        S_INSTALL_SQUASHFS_VERSION?=3
    else
        S_INSTALL_SQUASHFS_VERSION?=4
    endif
else
    S_INSTALL_SQUASHFS_VERSION?=4
endif

S_INSTALL_PAD_BLOCK_SIZE?=131072

#======== sysinit =========
S_INSTALL_UPGRADE_ARCH?=$(SRM_BOARD)
S_INSTALL_UPGRADE_IMG?=img
S_INSTALL_UPGRADE_PROJECT?=smp
S_INSTALL_UPGRADE_PLATFORM_SPEC?=$(SRM_BOARD)
S_INSTALL_UPGRADE_SECURE ?= yes

$(BUILDDIR)/.upgrade: $(BUILDDIR)/.configure
ifeq (yes,$(S_INSTALL_UPGRADE))
	make -C upgrade install
endif
	touch $(BUILDDIR)/.upgrade

$(BUILDDIR)/.configure:
	#configuration files
	make -C selectors install
	make -C config install
	touch $(BUILDDIR)/.configure

$(BUILDDIR)/.depmod:
	srm-host-run fwtools.in/run_depmod.sh

$(BUILDDIR)/.strip:
ifeq ($(S_INSTALL_DO_STRIP),yes)
	echo strip binaries and executables
	srm-host-run $(BUILD_ROOT)/bin/strip_ELF_files.py --target-only
	echo strip all modules
	find $(DESTDIR) -type f -name "*.ko*" -exec file {} \; | grep 'not stripped' | cut -d ':' -f 1 | xargs -r $(STRIP) --strip-unneeded || true
endif
	touch $(BUILDDIR)/.strip
ifneq ($(S_INSTALL_REMOVE_RESTRICTED),)
	@echo "Removing restricted directories"
	@for i in $$(echo ${S_INSTALL_REMOVE_RESTRICTED}); do echo Removing: $$i; rm -rf $$i; done;
endif

.licenses:
	$(BUILD_ROOT)/bin/check_licenses.py $(BUILD_ROOT)/usr/local/share/license_checker/db.json $(DESTDIR)/

.cerberus:
	@if [ -x $(BUILD_ROOT)/bin/cerberus-client.py ]; then\
		if [ $(S_SECURE_BOOT_DRYRUN) != "yes" ]; then\
			srm-host-run $(BUILD_ROOT)/bin/cerberus-client.py $(SRM_ROOT) $(BUILD_ROOT)/usr/local/share/cerberus-client/profiles.json;\
		else\
			echo "*** Cerberus check skipped since it's not a secure/production build";\
		fi;\
	else\
		echo "";\
		echo "***********************************************************************************************";\
		echo "***********************************************************************************************";\
		echo "Your sbuild is missing cerberus-client. You NEED to install it from: Src/cerberus-client/c.comp";\
		echo "***********************************************************************************************";\
		echo "***********************************************************************************************";\
		echo "";\
		exit 1;\
	fi;

$(BUILDDIR)/.apps:
	make -C apps install
	touch $(BUILDDIR)/.apps

$(BUILDDIR)/.sysinit: $(BUILDDIR)/.strip $(BUILDDIR)/.depmod $(BUILDDIR)/.upgrade $(BUILDDIR)/.configure $(BUILDDIR)/.certs $(BUILDDIR)/.apps .driver_variant
	make -C sysinit install
	touch $(BUILDDIR)/.sysinit

$(BUILDDIR)/.certs:
	make -C certs install
	touch $(BUILDDIR)/.certs

$(BUILDDIR)/.keys:
	make -C keys install
	touch $(BUILDDIR)/.keys

.driver_variant:
	@if [ -d $(SRM_ROOT)/target/drivers ]; then\
		echo "*** Found drivers folder, looking for [$(S_INSTALL_DRIVER_VARIANT)]";\
		if [ -d $(SRM_ROOT)/target/drivers/$(S_INSTALL_DRIVER_VARIANT) ]; then\
			echo "*** Found driver variant [$(S_INSTALL_DRIVER_VARIANT)]. Installing to target/root";\
			cp -rfv $(SRM_ROOT)/target/drivers/$(S_INSTALL_DRIVER_VARIANT)/* $(SRM_ROOT)/target/root;\
			rm -rf $(SRM_ROOT)/target/root/info.txt;\
		else\
			echo "*** Driver variant [$(S_INSTALL_DRIVER_VARIANT)] not found!. Please inspect your sbuild and/or project file.";\
			exit 1;\
		fi;\
	fi;
#==========================


#======== TARGET ==========
#TODO: develop rule for rootfs without /usr or something like /usr

SRCFS=$(shell mapreader.pl $(MAP) src dst=$(notdir $@))
SIZEFS=$(shell mapreader.pl $(MAP) size dst=$(notdir $@))
PADFS=$(shell mapreader.pl $(MAP) attr dst=$(notdir $@) | grep -o "zeropadding\|randompadding\|blockpadding\|qbimage" || echo nopadding)
ROOTFS=$(BUILDDIR)/.rootfs
ROOTFSTYPE=`echo "$(shell mapreader.pl $(MAP) attr src=/)" | grep -o -e "tgz\|cramfs\|squashfs"`
ROOTFS_FILE=$(shell mapreader.pl $(MAP) dst src=/)

ifeq ("$(S_INSTALL_VMX_UPGRADE_2)","yes")
    VMX_TARGET=$(TOOLSDIR)/mkimg.vmx.2.sh
else
    VMX_TARGET=$(TOOLSDIR)/mkimg.vmx.sh
endif

ifeq ("$(S_INSTALL_LATENS_UPGRADE_2)","yes")
    LATENS_TARGET=$(TOOLSDIR)/mkimg.latens.2.sh
else
    LATENS_TARGET=$(TOOLSDIR)/mkimg.latens.sh
endif

#FS_COMMON_DEPS=$(BUILDDIR)/.sysinit $(BUILDDIR)/.configure $(BUILDDIR)/.keys $(TOOLSDIR)/README.mkfs.sh

$(ROOTFS): $(BUILDDIR)/.sysinit $(BUILDDIR)/.configure $(BUILDDIR)/.keys $(TOOLSDIR)/README.mkfs.sh
ifeq (yes,$(S_INSTALL_USE_CERBERUS))
	make .cerberus
endif
	make .licenses
	rm -rf $@
	mkdir -p $(dir $@)
	cp -a $(DESTDIR) $@.dir
	mv $@.dir $@
ifeq (yes,$(S_SECURE_BOOT))
	chmod -R o-rwx $@
	chmod -R ug-w $@
	chmod -R ug+rwX $@/dev
	chown -R 0:1000 $@
	if [ -c $@/dev/watchdog ]; then chmod o+rw $@/dev/watchdog; fi
endif
	make -C hwstash

$(TOOLSDIR)/README.mkfs.sh:
	sed "s/_TOOL_NAME_/mkfs.sh.in/g" < fwtools.in/README.mkfs.sh.in > $@

$(TOOLSDIR)/mkfs.tgz.sh: fwtools.in/mkfs.sh.in
	cp fwtools.in/mkfs.sh.in $@.tmp
	sed -i 's#\bS_MK_FILE_SYSTEM_CMD\b#mkdir -p $$SRC ; cd $$SRC ; tar cf $$DST --use-compress-prog=pigz *#g' $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#mkdir -p $$DST ; tar xf $$SRC --use-compress-prog=pigz -C $$DST#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#not-implemented#g' $@.tmp
ifeq (usb,$(S_INSTALL_CONFIG_SH))
	sed -i 's#\bS_SKIP_SIZE_CHECK\b#yes#g' $@.tmp
endif
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst tgz)): $(TOOLSDIR)/mkfs.tgz.sh $(ROOTFS)
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkfs.tgz.sh $(ROOTFS)/$(SRCFS) $@ $(SIZEFS) $(PADFS) && rm -rf $(ROOTFS)/$(SRCFS)/*

$(TOOLSDIR)/mkfs.squashfs.sh: fwtools.in/mkfs.sh.in
	cp $< $@.tmp
ifeq ($(S_INSTALL_SQUASHFS_VERSION),4)
	sed -i 's#\bS_MK_FILE_SYSTEM_CMD\b#mksquashfs4 $$SRC/ $$DST -no-progress #g' $@.tmp
else
	sed -i 's#\bS_MK_FILE_SYSTEM_CMD\b#mksquashfs $$SRC/ $$DST -le -no-progress #g' $@.tmp
endif
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#unsquashfs -f -d $$DST $$SRC #g' $@.tmp
	sed -i 's#\bS_BLOCK_SIZE\b#$(S_INSTALL_PAD_BLOCK_SIZE)#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#$(S_SECURE_BOOT)#g' $@.tmp
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst squashfs)): $(TOOLSDIR)/mkfs.squashfs.sh $(ROOTFS)
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkfs.squashfs.sh $(ROOTFS)/$(SRCFS) $@ $(SIZEFS) $(PADFS) && rm -rf $(ROOTFS)/$(SRCFS)/*

$(TOOLSDIR)/mkimg.sec3.sh:
	cp fwtools.in/mkimg.sec3.in $@.tmp
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst sec3)): $(TOOLSDIR)/mkimg.sec3.sh $(ROOTFS)
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkimg.sec3.sh $(ROOTFS)/ $@ $(S_SECURE_BOOT_DRYRUN) $(DESTDIR)/../init && rm -rf $(ROOTFS)/*

$(TOOLSDIR)/mkfs.ubifs.sh: fwtools.in/mkfs.sh.in fwtools.in/mkfs.ubi.aes.sh
	cp fwtools.in/mkfs.sh.in $@.tmp
	install -m 0755 fwtools.in/mkfs.ubi.aes.sh $(TOOLSDIR)/
	sed -i 's#\bS_MK_FILE_SYSTEM_CMD\b#$(TOOLSDIR)/mkfs.ubi.aes.sh $$SRC $$DST $$UBIFS_MIN_IO_SIZE $$UBIFS_LEB_SIZE $$UBIFS_MAX_LEBS#g' $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#no#g' $@.tmp
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst ubifs)): $(TOOLSDIR)/mkfs.ubifs.sh $(ROOTFS)
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkfs.ubifs.sh $(ROOTFS)/$(SRCFS) $@ $(SIZEFS) $(PADFS) && rm -rf $(ROOTFS)/$(SRCFS)/*

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst squbifs)): $(TOOLSDIR)/mkfs.squashfs.sh $(ROOTFS)
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkfs.squashfs.sh $(ROOTFS)/$(SRCFS) $@ $(SIZEFS) blockpadding && rm -rf $(ROOTFS)/$(SRCFS)/*

$(TOOLSDIR)/mkimage_for_signing.sh: fwtools.in/mkimage_for_signing.sh.in
	cp $< $@.tmp
	sed "s#\bS_LATENS_OEM_NAME\b#$(S_LATENS_OEM_NAME)#g" -i $@.tmp
	sed "s#\bS_LATENS_STB_MODEL\b#$(S_LATENS_STB_MODEL)#g" -i $@.tmp
	cp $@.tmp $@

$(TOOLSDIR)/mkimg.vmx.sh: fwtools.in/mkimg.vmx.in
	cp $< $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#no#g' $@.tmp
	cp $@.tmp $@

$(TOOLSDIR)/mkimg.vmx.2.sh: fwtools.in/mkimg.vmx.2.in
	cp $< $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#no#g' $@.tmp
	cp $@.tmp $@

vmx_release: fwtools.in/vmx/README.vmx_release fwtools.in/packagegen_util.sh fwtools.in/qbnfsrootgen.sh fwtools.in/qbupgradegen.sh $(TOOLSDIR)/mkimage_for_signing.sh fwtools.in/nfsroot_install.sh
	install -m 755 -d $(DESTDIR)/../../vmx_release
	install -m 644 fwtools.in/vmx/README.vmx_release $(DESTDIR)/../../
	install -m 755 fwtools.in/qbnfsrootgen.sh $(DESTDIR)/../../vmx_release
	install -m 755 fwtools.in/qbupgradegen.sh $(DESTDIR)/../../vmx_release
	install -m 755 fwtools.in/packagegen_util.sh $(DESTDIR)/../../vmx_release
	install -m 755 $(TOOLSDIR)/mkimage_for_signing.sh $(DESTDIR)/../../vmx_release
	install -m 755 fwtools.in/nfsroot_install.sh $(DESTDIR)/../../vmx_release

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst vmximg)): $(VMX_TARGET) $(ROOTFS) vmx_release
	# S_BOOTIMAGE_SIZE_LIMIT is maximum BOOTIMAGE size in bytes. If it is not set in project file,
	# then it will be taken from memory map
	$(eval S_BOOTIMAGE_SIZE_LIMIT?=$(shell mapreader.pl $(MAP) size dst=$(notdir $@)))
	echo "Bootimage size is limited to $(S_BOOTIMAGE_SIZE_LIMIT)"
	cd $(BUILDDIR) && bash $(VMX_TARGET) $(DESTDIR)/../$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) $(ROOTFS)/ $@ $@.strip $(S_SECURE_BOOT_DRYRUN) $(DESTDIR)/../init $(S_BOOTIMAGE_SIZE_LIMIT) && rm -rf $(ROOTFS)/*
	mv $@.strip $(SRM_ROOT)/$(SRM_PROJECT)-$(VERSION)-$(notdir $@).stripped

$(TOOLSDIR)/mkimg.latens.sh: fwtools.in/mkimg.latens.in
	cp $< $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#no#g' $@.tmp
	cp $@.tmp $@

$(TOOLSDIR)/mkimg.latens.2.sh: fwtools.in/mkimg.latens.2.in
	cp fwtools.in/latens/qbheader.sh $(TOOLSDIR)/
	cp fwtools.in/latens/qbsignature.sh $(TOOLSDIR)/
	cp $< $@.tmp
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' $@.tmp
	sed -i 's#\bS_SECURE_BOOT\b#no#g' $@.tmp
	cp $@.tmp $@

latens_release: fwtools.in/latens/README.latens_release fwtools.in/latens/qbheader.sh fwtools.in/latens/qbsignature.sh fwtools.in/latens/Firmware_installation_instructions.txt fwtools.in/latens/release_notes.txt.template fwtools.in/packagegen_util.sh fwtools.in/qbnfsrootgen.sh fwtools.in/qbupgradegen.sh $(TOOLSDIR)/mkimage_for_signing.sh fwtools.in/nfsroot_install.sh
	install -m 755 -d $(DESTDIR)/../../latens_release
	install -m 644 fwtools.in/latens/README.latens_release $(DESTDIR)/../../
	install -m 755 fwtools.in/latens/qbheader.sh $(DESTDIR)/../../latens_release
	install -m 755 fwtools.in/latens/qbsignature.sh $(DESTDIR)/../../latens_release
	install -m 644 fwtools.in/latens/Firmware_installation_instructions.txt $(DESTDIR)/../../latens_release
	install -m 644 fwtools.in/latens/release_notes.txt.template $(DESTDIR)/../../latens_release
	install -m 755 fwtools.in/qbnfsrootgen.sh $(DESTDIR)/../../latens_release
	install -m 755 fwtools.in/qbupgradegen.sh $(DESTDIR)/../../latens_release
	install -m 755 fwtools.in/packagegen_util.sh $(DESTDIR)/../../latens_release
	install -m 755 $(TOOLSDIR)/mkimage_for_signing.sh $(DESTDIR)/../../latens_release
	install -m 755 fwtools.in/nfsroot_install.sh $(DESTDIR)/../../latens_release

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst latensimg)): $(LATENS_TARGET) $(ROOTFS) latens_release
ifeq ("$(S_INSTALL_LATENS_UPGRADE_2)","yes")
	cd $(BUILDDIR) && bash -x $(LATENS_TARGET) $(DESTDIR)/../$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) $(ROOTFS)/ $@ $@.strip $(S_SECURE_BOOT_DRYRUN) $(DESTDIR)/../init && rm -rf $(ROOTFS)/*
else
	cd $(BUILDDIR) && bash -x $(LATENS_TARGET) $(DESTDIR)/../$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) $(ROOTFS)/ $@ $(S_SECURE_BOOT_DRYRUN) $(DESTDIR)/../init && rm -rf $(ROOTFS)/*
endif

$(TOOLSDIR)/mkfs.cramfs.sh: fwtools.in/mkfs.sh.in
	cp $< $@.tmp
	sed -i 's#\bS_MK_FILE_SYSTEM_CMD\b#mkcramfs $$SRC $$DST #g' .script
	sed -i 's#\bS_UNPACK_FILE_SYSTEM_CMD\b#{ echo not yet supported; exit 1;}#g' .script
	sed -i 's#\bS_SECURE_BOOT\b#$(S_SECURE_BOOT)#g' .script
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst cramfs)): $(TOOLSDIR)/mkfs.cramfs.sh $(ROOTFS)
	@echo
	@echo cramfs is highly not recommended, use squashfs if you can!!!
	@echo
	@sleep 3
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkfs.cramfs.sh $(ROOTFS)/$(SRCFS) $@ $(SIZEFS) $(PADFS) && rm -rf $(ROOTFS)/$(SRCFS)/*

$(TOOLSDIR)/makeskin.sh: fwtools.in/makeskin.sh.in
	cp $< $@.tmp
	sed -i 's#\bS_VERSION\b#$(SRM_PROJECT)-$(VERSION)#g' $@.tmp
ifneq ($(S_INSTALL_USE_KEYSERVER),yes)
	$(error Skinning without keyserver support is not permitted)
endif
	sed -i 's#\b_REPLACE_S_INSTALL_KEYSERVER_RSA_SKIN_KEY_ID_\b#$(S_INSTALL_KEYSERVER_RSA_SKIN_KEY_ID)#g' $@.tmp
	sed -i 's#\b_REPLACE_S_INSTALL_KEYSERVER_RSA_SKIN_HASH_\b#$(S_INSTALL_KEYSERVER_RSA_SKIN_HASH)#g' $@.tmp
	sed -e "s/_TOOL_NAME_/$(notdir $@)/g" \
		< fwtools.in/README.$(notdir $@).in > $(TOOLSDIR)/README.$(notdir $@)
	cp $@.tmp $@

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst binary)):
	if [ "$(shell mapreader.pl $(MAP) dst dst=$(notdir $@),3des_encrypt)" = "$(notdir $@)" ] && [ "$(S_KERNEL_USE_KEYCHAIN)" = "no" ]; then \
		if [ "$(S_INSTALL_USE_KEYSERVER)" = "yes" ]; then \
			srm-host-run jclient encrypt -t $(DESTDIR)/../$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) with $(S_INSTALL_KEYSERVER_SYM_KERNEL_KEY_ID) > $@ || exit $?;\
		else \
			echo "Kernel encryption without Keyserver is not supported"; exit 1; \
		fi \
	else \
		cp -f $(DESTDIR)/../$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) $@; \
	fi
	[ `wc -c $@ | sed "s/\([[:digit:]]*\).*/\1/"` -le $$(( $(shell mapreader.pl $(MAP) size dst=$(notdir $@)) )) ] || { echo binary $@ is too BIG; exit 1; }

$(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst bootcfg)):
	cp -f bootcfg/$(shell mapreader.pl $(MAP) src dst=$(notdir $@)) $@
	[ `wc -c $@ | sed "s/\([[:digit:]]*\).*/\1/"` -le $$(( $(shell mapreader.pl $(MAP) size dst=$(notdir $@)) )) ] || { echo bootcfg $@ is too BIG; exit 1; }

#add more binaries here (bootloader, logo, config, ...)
#==========================


#======== UPGRADE =========
UPGRADE_PACK = $(BUILDDIR)/$(UPGRADE)-$(VERSION).tgz

UPGRADE_FILES=$(shell mapreader.pl $(MAP) dst upgrade)
S_INSTALL_UPGRADE_ARCH?=$(SRM_BOARD)
S_INSTALL_UPGRADE_IMG?=img
S_INSTALL_UPGRADE_PROJECT?=smp
FW_NAME = $(S_INSTALL_UPGRADE_ARCH)-$(S_INSTALL_UPGRADE_IMG)-$(S_INSTALL_UPGRADE_PROJECT)

export

$(UPGRADE_PACK): $(addprefix $(BUILDDIR)/,$(UPGRADE_FILES))
	cd $(BUILDDIR) && bash $(TOOLSDIR)/mkupgrade.sh . $(notdir $@)

$(TOOLSDIR)/mkupgrade.sh:
	mkdir -p $(TOOLSDIR)
	cp fwtools.in/$(notdir $@).in $@.tmp
ifeq ($(S_INSTALL_UPGRADE_SECURE),yes)
ifeq ($(S_INSTALL_LEGACY_ECDSA_UPGRADE),yes)
	srm-local-run gcc -Wall fwtools.in/sign.c -o sign -static -lcrypto -ldl || srm-local-run gcc -Wall fwtools.in/sign.c -o sign -static -lcrypto -ldl -m32
	cp $(S_INSTALL_UPPCERT_DIR)/supplier.cert.der ./
	cp $(S_INSTALL_UPPCERT_DIR)/supplier.key.der ./
endif
endif
	sed "s#\bS_UPGRADE_FILES\b#$(UPGRADE_FILES)#g" -i $@.tmp
	sed "s#\bS_FW_NAME\b#$(FW_NAME)#g" -i $@.tmp
	sed "s#\bS_RELEASE\b#$(RELEASE)#g" -i $@.tmp
	sed "s#\bS_SECURE_BOOT\b#$(S_SECURE_BOOT)#g" -i $@.tmp
	sed "s#\bS_SECURE_UPGRADE\b#$(S_INSTALL_UPGRADE_SECURE)#g" -i $@.tmp
	sed "s#\b_S_INSTALL_KEYSERVER_RSA_BOOT_HASH_\b#$(S_INSTALL_KEYSERVER_RSA_BOOT_HASH)#g" -i $@.tmp
	sed "s#\b_S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH_\b#$(S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH)#g" -i $@.tmp
	sed "s#\bS_INSTALL_LATENS_UPGRADE\b#$(S_INSTALL_LATENS_UPGRADE)#g" -i $@.tmp
	sed "s#\bS_INSTALL_VMX_UPGRADE\b#$(S_INSTALL_VMX_UPGRADE)#g" -i $@.tmp
	rm -f $(BUILDDIR)/.list
	for i in $(UPGRADE_FILES); do \
		echo -e "\t$$i - "`mapreader.pl $(MAP) comment dst=$$i` >> $(BUILDDIR)/.list; done
	sed -e "/_SRC_FILE_LIST_/r $(BUILDDIR)/.list" -e "/_SRC_FILE_LIST_/d" \
		-e "s/_TOOL_NAME_/$(notdir $@)/g" \
		< fwtools.in/README.$(notdir $@).in > $(TOOLSDIR)/README.$(notdir $@)
	mv $@.tmp $@
ifeq ($(S_INSTALL_LATENS_UPGRADE),yes)
	install -m 755 -d $(DESTDIR)/../../latens_release
	install -m 755 $@ $(DESTDIR)/../../latens_release
endif
ifeq ($(S_INSTALL_VMX_UPGRADE),yes)
	install -m 755 -d $(DESTDIR)/../../vmx_release
	install -m 755 $@ $(DESTDIR)/../../vmx_release
endif

#==========================

#======== NFSROOT =========
NFSROOT_PACK = $(BUILDDIR)/$(NFSROOT)-$(VERSION).tgz

$(TOOLSDIR)/install.%.sh.in: $(addprefix $(BUILDDIR)/,$(shell mapreader.pl $(MAP) dst fwimage))
	mkdir -p $(DESTDIR)/firmware/src
	rm -f $@.tmp
	touch $@.tmp
ifeq ($(notdir $@),install.disk.sh.in)
	echo "../etc/init.d/do_mkpart" >> $@.tmp
endif
	#install binaries
	for id in $(shell mapreader.pl $(MAPDIR)/$*.map id fwimage); do \
		eval $$(mapreader.pl $(MAPDIR)/$*.map "*" id=$$id); \
		install -m 644 $(BUILDDIR)/$$dst $(DESTDIR)/firmware/src/$$dst || exit 1;\
		if [ "$(notdir $@)" = "install.nor.sh.in" -a ! -z "${S_INSTALL_SH_NOR_MAPPED}" ]; then \
			echo "do_mapped_nor_write src/$$dst $$offset $$size" >> $@.tmp; \
		elif [ "$(notdir $@)" = "install.nor.sh.in" -o "$(notdir $@)" = "install.nand.sh.in" -o "$(notdir $@)" = "install.spi.sh.in" ]; then \
			echo "do_named_mtd_write src/$$dst $$name '$$attr'" >> $@.tmp; \
		elif [ "$(notdir $@)" = "install.mmc.sh.in" ]; then \
			echo "do_named_mmc_write src/$$dst $$name" >> $@.tmp; \
		else \
			echo "do_named_part_write src/$$dst $$name $$size" >> $@.tmp; \
		fi \
	done
	#erase some partitions
	for name in $(shell mapreader.pl $(MAPDIR)/$*.map name fwerase); do \
		echo "do_flash_erase $$name" >> $@.tmp; \
	done
	cp -f $@.tmp $@

ifeq ($(S_SECURE_BOOT),yes)
NFSROOT_SECURES=$(UPGRADE_PACK) ${FW_NAME}.sig
endif

NFSROOT_INSTALLERS=$(patsubst %.map,$(TOOLSDIR)/install.%.sh.in,$(notdir $(wildcard $(MAPDIR)/*.map)))

external-release-nfsroot.tgz:
ifdef S_INSTALL_NFSROOT_EXTERNAL_RELEASE
	rm -rf external-release
	mkdir external-release
	if [ ! -f $(S_INSTALL_NFSROOT_EXTERNAL_RELEASE) ]; then \
		cd external-release && silo pull sbuild $(S_INSTALL_NFSROOT_EXTERNAL_RELEASE) && \
		mv external-release/sbuild-*/*-nfsroot-*.tgz nfsroot-external-release.tgz;\
	else\
		cp -f $(S_INSTALL_NFSROOT_EXTERNAL_RELEASE) external-release-nfsroot.tgz;\
	fi
	rm -rf external-release
endif

$(NFSROOT_PACK): $(BUILDDIR)/.sysinit $(BUILDDIR)/.configure $(NFSROOT_INSTALLERS) external-release-nfsroot.tgz
ifndef S_INSTALL_NFSROOT_EXTERNAL_RELEASE
	mkdir -p $(DESTDIR)/firmware/src
	rm -f $(DESTDIR)/firmware/install.sh $(DESTDIR)/firmware/install.sh.tmp
	if [ ! -e install.sh/platforms/$(S_INSTALL_UPGRADE_PLATFORM_SPEC) ]; then \
		cat install.sh/generic.sh.in install.sh/platforms/generic $(NFSROOT_INSTALLERS) > $(DESTDIR)/firmware/install.sh.tmp; \
	else\
		cat install.sh/generic.sh.in install.sh/platforms/$(S_INSTALL_UPGRADE_PLATFORM_SPEC) $(NFSROOT_INSTALLERS) > $(DESTDIR)/firmware/install.sh.tmp; \
	fi
  ifeq ($(S_SECURE_BOOT),yes)
    ifneq (,$(filter $(S_INSTALL_UPGRADE_PLATFORM_SPEC),bcm97xxx bcm97xxx-mmc))
	echo set_env_var SV_SLOT 0 >> $(DESTDIR)/firmware/install.sh.tmp
	echo set_env_var SV_VERSION FLASHED >> $(DESTDIR)/firmware/install.sh.tmp
	echo set_env_var SV_STATUS FLASHED >> $(DESTDIR)/firmware/install.sh.tmp
    else
	echo set_env_var qb_slot 0 >> $(DESTDIR)/firmware/install.sh.tmp
	echo set_env_var qb_version FLASHED >> $(DESTDIR)/firmware/install.sh.tmp
	echo set_env_var qb_status FLASHED >> $(DESTDIR)/firmware/install.sh.tmp
    endif
    ifeq (,$(filter yes, $(S_INSTALL_VMX_UPGRADE) $(S_INSTALL_LATENS_UPGRADE)))
      ifeq ($(shell mapreader.pl $(MAP) name qbimage | grep -o rootfs0),)
	echo $(shell grep ROOTFS. $(BUILDDIR)/${FW_NAME}.sig | sed -e 's/ROOTFS.*=/set_env_var rootfs0_sign /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep ROOTFS. $(BUILDDIR)/${FW_NAME}.sig | sed -e 's/ROOTFS.*=/set_env_var rootfs1_sign /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep ROOTFS. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/ROOTFS.*=/set_env_var rootfs0_size /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep ROOTFS. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/ROOTFS.*=/set_env_var rootfs1_size /') >> $(DESTDIR)/firmware/install.sh.tmp
      endif
      ifeq ($(shell mapreader.pl $(MAP) name qbimage | grep -o vmlinuz0),)
	echo $(shell grep KERNEL. $(BUILDDIR)/${FW_NAME}.sig | sed -e 's/KERNEL.*=/set_env_var vmlinuz0_sign /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep KERNEL. $(BUILDDIR)/${FW_NAME}.sig | sed -e 's/KERNEL.*=/set_env_var vmlinuz1_sign /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep KERNEL. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/KERNEL.*=/set_env_var vmlinuz0_size /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep KERNEL. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/KERNEL.*=/set_env_var vmlinuz1_size /') >> $(DESTDIR)/firmware/install.sh.tmp
      endif
      ifeq ($(shell mapreader.pl $(MAP) name name=see_code0),)
	echo $(shell grep SEECODE. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/SEECODE.*=/set_env_var see0_size /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep SEECODE. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/SEECODE.*=/set_env_var see1_size /') >> $(DESTDIR)/firmware/install.sh.tmp
      endif
    endif
    ifeq ($(shell mapreader.pl $(MAP) name qbimage | grep -o logo),)
	echo $(shell grep LOGO. $(BUILDDIR)/${FW_NAME}.sig | sed -e 's/LOGO.*=/set_env_var splash_sign /') >> $(DESTDIR)/firmware/install.sh.tmp
	echo $(shell grep LOGO. $(BUILDDIR)/${FW_NAME}.size | sed -e 's/LOGO.*=/set_env_var splash_size /') >> $(DESTDIR)/firmware/install.sh.tmp
    endif
  endif
	echo "go_out" >> $(DESTDIR)/firmware/install.sh.tmp
	chmod 755 $(DESTDIR)/firmware/install.sh.tmp
	mv $(DESTDIR)/firmware/install.sh.tmp $(DESTDIR)/firmware/install.sh
else
	cd $(DESTDIR)/ && tar xvf $(HERE)/external-release-nfsroot.tgz --use-compress-prog=pigz firmware > ~/fff.txt  2>&1; \
	echo $(S_INSTALL_NFSROOT_EXTERNAL_RELEASE) > touch $(DESTDIR)/firmware/this_is_external_firmware
endif
ifeq (yes,$(S_SECURE_BOOT))
	chmod -R o-rwx $(DESTDIR)
	chmod -R ug-w $(DESTDIR)
	chmod -R ug+rwX $(DESTDIR)/dev
	chown -R 0:1000 $(DESTDIR)
	if [ -c $(DESTDIR)/dev/watchdog ]; then chmod o+rw $(DESTDIR)/dev/watchdog; fi
endif
	cd $(DESTDIR) && tar cf $@ --use-compress-prog=pigz *
	rm -rf $(DESTDIR)/firmware
#==========================

#======== SKINTOOLS =========
SKINTOOLS_PACK = $(BUILDDIR)/$(SKINTOOLS)-$(VERSION).tgz
SKINTOOLS_DIR = $(BUILDDIR)/$(SKINTOOLS)-$(VERSION)

$(SKINTOOLS_PACK): $(TOOLSDIR)/makeskin.sh $(ROOTFS)
	install -m 0755 -d $(SKINTOOLS_DIR)
	install -m 0755 -d $(SKINTOOLS_DIR)/skindata_orig
	install -m 0755 $(TOOLSDIR)/makeskin.sh $(SKINTOOLS_DIR)/
	install -m 0644 $(TOOLSDIR)/README.makeskin.sh $(SKINTOOLS_DIR)/
	cp -ar $(DESTDIR)/usr/local/share/CubiTV/data/. $(SKINTOOLS_DIR)/skindata_orig/
	chmod -R +w $(SKINTOOLS_DIR)/skindata_orig/
	find $(SKINTOOLS_DIR)/skindata_orig/ -type f -exec cat \{\} \; | md5sum | cut -d' ' -f 1 >$(SKINTOOLS_DIR)/orig.md5
	tar -C $(dir $(SKINTOOLS_DIR)) -cf $@ --use-compress-prog=pigz ./$(notdir $(SKINTOOLS_DIR))
	rm -rf $(SKINTOOLS_DIR)
#==========================


#======== TFTPBIN =========
TFTPBIN_PACK = $(TFTPBIN)-$(VERSION).bin
$(TFTPBIN_PACK): $(NFSROOT_PACK)
	make -C tftpbin install
	install -m 644 tftpbin/tftp.bin $@
#==========================


#======== PACKAGE =========
PACKAGE_PACK = $(PACKAGE)-$(VERSION).tgz
$(PACKAGE_PACK):
	mkdir -p package
	for i in $(shell mapreader.pl $(MAP) src package) ; do\
		install -m 0644 $(DESTDIR)/$$i ./package; done
	tar cvf $@ --use-compress-prog=pigz ./package
#==========================

#======= DRIVER ============
DRIVER_PACK = $(BUILDDIR)/$(DRIVER)-$(VERSION).tgz

DRIVER_FILES_BRCM = host/cross/mipsel-linux-uclibc/sys-root/usr/local/lib/libnexus.so\
target/root/lib/libnexus.so\
target/root/lib/modules/*/nexus/nexus.ko

DRIVER_FILES_ST = host/cross/sh4-linux/usr/lib/libstapi.so\
host/cross/sh4-linux/usr/local/lib/libstapi.so\
host/cross/sh4-linux/usr/lib/libstapp.so\
host/cross/sh4-linux/usr/local/lib/libstapp.so\
target/root/usr/lib/libstapi.so\
target/root/usr/local/lib/libstapi.so\
target/root/usr/lib/libstapp.so\
target/root/usr/local/lib/libstapp.so\
target/root/modules_mb680_7105_ST40_LINUX_32BITS\
target/root/modules_hdkh225_7141_ST40ESTB_LINUX_32BITS

DRIVER_FILES_ALI = host/cross/arm-linux/usr/lib/libaui.so

DRIVER_FILES_MSTAR = host/cross/mips-linux/usr/lib/libapiAUDIO.so

DRIVER_FILES_COMMON = target/drivers/*

# Now some heuristics to detect platform type
ifneq ($(S_SEECODE_PACK),)
DRIVER_FILES := $(DRIVER_FILES_ALI) $(DRIVER_FILES_COMMON)
DRIVER_INFO := Ali/AUI System Detected
endif

ifeq ($(S_STB_VENDOR),mstar)
DRIVER_FILES := $(DRIVER_FILES_MSTAR) $(DRIVER_FILES_COMMON)
DRIVER_INFO := MSTAR/MI System Detected
endif

ifeq ($(ARCH),sh4)
DRIVER_FILES := $(DRIVER_FILES_ST) $(DRIVER_FILES_COMMON)
DRIVER_INFO := STMicro/SH4 system detected
endif

ifneq ($(NEXUS_MODE),)
DRIVER_FILES := $(DRIVER_FILES_BRCM) $(DRIVER_FILES_COMMON)
DRIVER_INFO := Broadcom/Nexus system detected
endif

ifeq ($(DRIVER_FILES),)
$(error Could not detect SoC manufacturer. You need to add detection heuristics first... )
endif

ifeq ($(wildcard $(addprefix $(SRM_ROOT)/,$(DRIVER_FILES))),)
$(info *** $(DRIVER_INFO))
$(error Driver files NOT FOUND, likely you need to run Src/driver-installer/c.comp first)
endif

$(DRIVER_PACK):
	@echo \*** $(DRIVER_INFO)
	@echo "*** Creating DRIVER pack from files: $(DRIVER_FILES)"
	cd $(DESTDIR)/../.. && find $(DRIVER_FILES) -print0 | sort -z | cpio --create --null --format=ustar | gzip > $@
	tar -xzf $@ -s -O | md5sum -b | cut -d' ' -f 1 >$(BUILDDIR)/.driver_hash
	if [ `cat $(BUILDDIR)/.driver_hash` == "d41d8cd98f00b204e9800998ecf8427e" ]; then echo "*** Driver pack is empty, do something about it first!"; exit 1; fi
	@echo "SRM_DRIVER_PACK=$(DRIVER)-`cat $(BUILDDIR)/.driver_hash`.tgz" >> $(SRM_ENV)

ifeq ($(S_INSTALL_UPGRADE),yes)
HARVEST += $(UPGRADE_PACK)
endif
ifeq ($(S_INSTALL_NFSROOT),yes)
HARVEST += $(NFSROOT_PACK)
endif
ifeq ($(S_INSTALL_SKINTOOLS),yes)
HARVEST += $(SKINTOOLS_PACK)
endif
ifeq ($(S_INSTALL_PACKAGE),yes)
HARVEST += $(PACKAGE_PACK)
endif
ifeq ($(S_INSTALL_TFTPBIN),yes)
HARVEST += $(TFTPBIN_PACK)
endif
#ifeq ($(S_INSTALL_DRIVER),yes)
#HARVEST += $(DRIVER_PACK)
#endif


#dependencies which should be met under real user
compile: $(S_INSTALL_NFSROOT_EXTERNAL_RELEASE) $(TOOLSDIR)/mkupgrade.sh fakeroot_test umask_test
	make -C upgrade compile
	fakeroot make compile-root
ifeq ($(S_INSTALL_DRIVER),yes)
	make $(DRIVER_PACK)
endif

compile-root: $(MAP) $(HARVEST)

install:
	echo $(HARVEST)
	echo $(shell pwd)
	cp -f $(HARVEST) $(DESTDIR)/../../
ifeq ($(S_INSTALL_DRIVER),yes)
	cp $(DRIVER_PACK) $(DESTDIR)/../../
	cp $(DRIVER_PACK) $(DESTDIR)/../../$(DRIVER)-`cat $(BUILDDIR)/.driver_hash`.tgz
endif

.PHONY: fakeroot_test
fakeroot_test:
	srm-host-run fwtools.in/fakeroot_test.sh

.PHONY: umask_test
umask_test:
	srm-host-run fwtools.in/umask_test.sh

clean:
	rm -rf $(DESTDIR)/firmware
	rm -rf $(BUILDDIR)
