S_INSTALL_DYNAMIC_DEFUSER?=admin
S_INSTALL_DYNAMIC_DEFPASS?=admin
S_INSTALL_RCS_SCRIPT?=rcS
S_INSTALL_SYSINIT_SCRIPT?=sysinit
# by default use midnight, please (any date is fine)
S_INSTALL_INITIAL_DATE?=100700002011
S_SECURE_BOOT_DRYRUN?=no
S_LATENS_LIB_TYPE?=none
# Using 0 as default IGMP version - meaning highest supported by kernel
S_INSTALL_IGMP_PROTO_VERSION?=0

ifeq ($(S_INSTALL_MULTI_USER),yes)
	S_INSTALL_INITSCRIPTS?=upgrade gui hostname multiuser
else
	S_INSTALL_INITSCRIPTS?=upgrade gui hostname
endif

compile:

clean:

install:
	mkdir -p $(DESTDIR)/etc
	mkdir -p $(DESTDIR)/etc/init.d
	mkdir -p $(DESTDIR)/etc/upgrade.d
	mkdir -p $(DESTDIR)/etc/rcS.d
	mkdir -p $(DESTDIR)/etc/gui.d
	mkdir -p $(DESTDIR)/etc/platform.d
	install -m 0755 $(S_INSTALL_INITSCRIPTS) $(DESTDIR)/etc/init.d/
ifeq ($(S_SECURE_BOOT_DRYRUN),yes)
	install -m 0755 usbserial $(DESTDIR)/etc/init.d/
else
  ifeq ($(S_LATENS_LIB_TYPE),field-debug)
		install -m 0755 usbserial-notty $(DESTDIR)/etc/init.d/usbserial
  endif
endif
	install -m 0755 $(S_INSTALL_SYSINIT_SCRIPT) $(DESTDIR)/etc/init.d/sysinit
	install -m 0755 debug resume suspend $(DESTDIR)/etc/platform.d/
	install -m 0755 load.iptables load.nls factory_reset $(DESTDIR)/etc/init.d/
	install -m 0755 $(S_INSTALL_RCS_SCRIPT) $(DESTDIR)/etc/rcS
	sed -i -e 's/INITIAL_DATE/$(S_INSTALL_INITIAL_DATE)/' $(DESTDIR)/etc/rcS
	sed -i -e 's/IGMP_PROTO_VERSION/$(S_INSTALL_IGMP_PROTO_VERSION)/' $(DESTDIR)/etc/rcS
	install -m 0755 shell $(DESTDIR)/etc
	install -m 0755 soft-reboot.sh $(DESTDIR)/usr/local/bin/
	# Install gui scripts
	install -m 0755 gui_start $(DESTDIR)/etc/
	mkdir -p $(DESTDIR)/usr/local/bin
	install -m 0755 start_gui stop_gui restart_gui $(DESTDIR)/usr/local/bin/
	mkdir -p $(DESTDIR)/sbin #and do not overwrite poweroff/shutdown if exists
	[ -e $(DESTDIR)/sbin/poweroff ] || install -m 0755 poweroff $(DESTDIR)/sbin/
	[ -e $(DESTDIR)/sbin/shutdown ] || install -m 0755 shutdown $(DESTDIR)/sbin/
