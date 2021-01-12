
###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
# use only plaform specific functions for your implementation #
###############################################################

S_INSTALL_ISSUE?="Sentivision Multimedia Platform"
S_INSTALL_DISABLE_TELNETD?=no
S_INSTALL_DISABLE_SSHD?=no
S_INSTALL_MKPART?=configurable
S_INSTALL_SVSTORAGE_CONFIG?=
S_INSTALL_DISK_HDX?=
S_INSTALL_POLL_USB_DISK?=no
S_INSTALL_MULTI_USER?=no

ifeq ($(RCS_BLACKLIST),)				#if not defined in .prj file
ifeq ($(S_SECURE_BOOT_DRYRUN), yes)
	RCS_BLACKLIST=rcs_blacklist_debug
else
	RCS_BLACKLIST=rcs_blacklist_secure
endif
else
$(info Using RCS_BLACKLIST from project file, $(RCS_BLACKLIST))
endif

clean:
	make -C network clean
	rm -f sed.bak

install:
	mkdir -p $(DESTDIR)/opt/fake
	mkdir -p $(DESTDIR)/proc
	mkdir -p $(DESTDIR)/sys
	mkdir -p $(DESTDIR)/dev
	tar -C $(DESTDIR) --no-same-owner -xzpf $(S_INSTALL_BSPEC)/dev.tgz >/dev/null
	rm -f $(DESTDIR)/dev/kmem
	mkdir -p $(DESTDIR)/etc
	mkdir -p $(DESTDIR)/mnt
	mkdir -p $(DESTDIR)/home
	mkdir -p $(DESTDIR)/storage
	mkdir -p $(DESTDIR)/shared
	rm -f $(DESTDIR)/tmp
	ln -sf /dev/shm $(DESTDIR)/tmp
	ln -sf /tmp/var $(DESTDIR)/var
	ln -sf /tmp/root $(DESTDIR)/root
	ln -sf /tmp/vod $(DESTDIR)/etc/vod
	ln -sf /proc/mounts $(DESTDIR)/etc/mtab
	ln -sf /tmp/resolv.conf $(DESTDIR)/etc/resolv.conf
	ln -sf /shared/wwwroot $(DESTDIR)/wwwroot
	ln -sf /shared/contents $(DESTDIR)/contents
	#firmware identification files
	echo -n $(SRM_BOARD) > $(DESTDIR)/etc/BOARD
	echo -n $(SRM_PROJECT) > $(DESTDIR)/etc/BUILD
	echo $(SRM_RELEASE) > $(DESTDIR)/etc/RELEASE
	echo $(SRM_RELEASE) > $(DESTDIR)/etc/VERSION
	echo $(SRM_RELEASE_CANDIDATE) > $(DESTDIR)/etc/RC
	mkdir -p $(DESTDIR)/etc/base/
	echo -n $(SRM_RELEASE)`[ ! -z "$(SRM_RELEASE_CANDIDATE)" ] && echo rc$(SRM_RELEASE_CANDIDATE)` > $(DESTDIR)/etc/base/$(SRM_PROJECT)
	echo -n "${SRM_PROJECT}-${SRM_RELEASE}" > $(DESTDIR)/etc/SRM_PROJECT
	#a set needed for upgrade
	echo -n $(S_INSTALL_UPGRADE_ARCH) > $(DESTDIR)/etc/ARCH
	echo -n $(S_INSTALL_UPGRADE_IMG) > $(DESTDIR)/etc/IMG
	echo -n $(S_INSTALL_UPGRADE_PROJECT) > $(DESTDIR)/etc/PROJECT
ifeq ($(SRM_BUILD_ID),)
	# pysilo explicitly sets this variable to an empty value for non-tagged
	# project builds and when single component is built
	rm -f $(DESTDIR)/etc/SRM_BUILD_ID
else
	# this variable is set by pysilo when building tagged firmware version
	echo -n "$(SRM_BUILD_ID)" > $(DESTDIR)/etc/SRM_BUILD_ID
endif
	#install sub components of sysinit
	rm -f $(DESTDIR)/etc/passwd
	cp -f etc/* $(DESTDIR)/etc/
ifeq ($(S_SECURE_BOOT_DRYRUN), yes)
	echo "admin:!:0:0:root:/root:/bin/sh" >>$(DESTDIR)/etc/passwd
endif
	if [ -f $(S_INSTALL_BSPEC)/inittab ]; then\
		echo "Using bSpec's custom inittab..." ;\
		cp -f $(S_INSTALL_BSPEC)/inittab $(DESTDIR)/etc/ ;\
	else \
		echo "Using bInstall's generic inittab..." ;\
		cp -f etc/inittab $(DESTDIR)/etc/ ;\
	fi
	$(MAKE) -C scripts install
	$(MAKE) -C network install
	echo $(ISSUE) > $(DESTDIR)/etc/issue.net
	#mkpart
ifneq ($(S_INSTALL_MKPART),)
ifneq ($(S_INSTALL_DISK_HDX),)
	$(error S_INSTALL_DISK_HDX is no longer supported!!!)
endif
	cp mkpart/$(S_INSTALL_MKPART) $(DESTDIR)/etc/init.d/do_mkpart
	sed -e "s/@WAIT_FOR_USB_DISK@/$(S_INSTALL_POLL_USB_DISK)/g" \
		-i $(DESTDIR)/etc/init.d/do_mkpart
	echo -e "#!/bin/sh\n\n/etc/init.d/do_mkpart &\n\nexit 0" \
		> $(DESTDIR)/etc/init.d/mkpart
	chmod a+x $(DESTDIR)/etc/init.d/*mkpart
endif
	cp $(S_INSTALL_BSPEC)/mkpart.conf $(DESTDIR)/etc/mkpart.conf
	cp -r $(S_INSTALL_BSPEC)/profile.d $(DESTDIR)/etc

	#svstorage config file
ifneq ($(S_INSTALL_SVSTORAGE_CONFIG),)
ifneq ($(S_INSTALL_DISK_HDX),)
	$(error S_INSTALL_DISK_HDX is no longer supported!!!)
endif
	cp $(S_INSTALL_BSPEC)/storage.conf $(DESTDIR)/etc/storage.conf
endif
	#rcS.d
	rm -rf $(DESTDIR)/etc/rcS.d
	mkdir -p $(DESTDIR)/etc/rcS.d
	cat $(S_INSTALL_BSPEC)/rcS |  sed "s/#.*$$//" | while read order service; do\
		echo "$(S_INSTALL_RC_EXCLUDE)" | grep -q "\<$$service\>" && continue;\
	    if [ -f $(DESTDIR)/etc/init.d/$$service ]; then\
			ln -sf ../init.d/$$service $(DESTDIR)/etc/rcS.d/$$order$$service;\
	    fi;\
	done
	echo removing scripts
	# remove all unsused init.d scripts
	cat $(RCS_BLACKLIST) | while read S; do\
		REMOVE=1;\
		for F in $(DESTDIR)/etc/rcS.d/*; do\
			if [ "$$(basename $$(readlink $$F))" = "$$S" ]; then\
				REMOVE=0; break;\
			fi;\
		done;\
		if [ $$REMOVE -eq 1 ]; then\
			echo Removing $(DESTDIR)/etc/init.d/$$S;\
			rm -f $(DESTDIR)/etc/init.d/$$S ;\
		fi;\
	done
	#disable services in /etc/inetd.conf
ifeq ($(S_INSTALL_DISABLE_TELNETD),yes)
ifeq (exists,$(shell [ -f $(DESTDIR)/etc/inetd.conf ]) && echo exists )
	sed "/telnet/d" $(DESTDIR)/etc/inetd.conf > sed.bak
	mv sed.bak $(DESTDIR)/etc/inetd.conf
endif
endif
ifeq ($(S_INSTALL_DISABLE_SSHD),yes)
ifeq (exists,$(shell [ -f $(DESTDIR)/etc/inetd.conf ]) && echo exists )
	sed "/dropbear/d" $(DESTDIR)/etc/inetd.conf | sed "/sshd/d" > sed.bak
	mv sed.bak $(DESTDIR)/etc/inetd.conf
endif
endif

