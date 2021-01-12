
###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
# use only plaform specific functions for your implementation #
###############################################################

# All timeout values are expressed in seconds
S_INSTALL_UPGRADE_SCRIPT?=upgrade.sh
S_INSTALL_UPGRADE_CONFIRMATION_DELAY?=30
S_INSTALL_UPGRADE_SECURE_SCRATCH ?= 0
S_INSTALL_UPGRADE_PLATFORM_PROC ?= upgrade.proc
S_INSTALL_UPGRADE_OK ?= upgrade_ok.sh
S_INSTALL_USE_IMAGE_SIZES?=no
S_INSTALL_UPGRADE_RETRY_TIMEOUT?=30
S_INSTALL_UPGRADE_CONNECT_TIMEOUT?=30
S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH?=sha1
S_INSTALL_KEYSERVER_RSA_BOOT_HASH?=sha1
export S_INSTALL_UPGRADE_PLATFORM_PROC

SPECFILE=_upgrade_platform_spec.proc
export SPECFILE

# Using Verimatrix upgrade with reencryption
S_INSTALL_VMX_UPGRADE?=no

# Using Latens upgrade
S_INSTALL_LATENS_UPGRADE?=no

# Force platform initialization
S_INSTALL_UPGRADE_FORCE_PLATFORM_INIT?=no

# Wait for network before trying to upgrade
S_INSTALL_UPGRADE_WAIT_FOR_NETWORK?=no

# Wait for upgrade server before trying to upgrade
S_INSTALL_UPGRADE_WAIT_FOR_UPGRADE_SERVER?=no


ifeq (,$(S_INSTALL_UPGRADE_PLATFORM_SPEC))
$(error S_INSTALL_UPGRADE_PLATFORM_SPEC is not defined!)
endif

ifeq ($(S_INSTALL_UPGRADE_SECURE_SIGMA_XPU),yes)
CHECKSIGN_EXTRA=-DUSE_SIGMA_XPU -lxrpc -lmrua_impl -lllad -lrmcw -lrmcore
endif

ifneq ($(SV_LOG_LEVEL),)
  CHECKSIGN_EXTRA += -DSV_LOG_LEVEL=$(SV_LOG_LEVEL)
endif

ifeq (yes,$(S_INSTALL_USE_RSA_UPGRADE))
S_INSTALL_UPGRADE_SCRIPT=upgrade_rsa.sh
else ifeq (no,$(S_INSTALL_LEGACY_ECDSA_UPGRADE))
  $(error You have chosen NOT to use RSA upgrade, but you have NOT set S_INSTALL_LEGACY_ECDSA_UPGRADE=yes as well!. Fix your .prj files)
endif

all: clean install

compile:
ifeq ($(S_INSTALL_UPGRADE_SECURE),yes)
ifeq ($(S_INSTALL_LEGACY_ECDSA_UPGRADE),yes)
	gcc $(CHECKSIGN_EXTRA) -lcrypto -L$(CROSS_ROOT)/usr/local/lib checksign.c -o $(DESTDIR)/bin/checksign
endif
	gcc -L$(CROSS_ROOT)/usr/local/lib xxd.c -o $(DESTDIR)/bin/xxd
	#ca is installed in keys submodule
endif
	make -C $(S_INSTALL_UPGRADE_PLATFORM_SPEC) compile

install:
	mkdir -p $(DESTDIR)/bin
	install $(S_INSTALL_UPGRADE_SCRIPT) $(DESTDIR)/bin/upgrade
	sed -i "s/_SECURE_UPGRADE_/$(S_INSTALL_UPGRADE_SECURE)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_SECURE_SCRATCH_/$(S_INSTALL_UPGRADE_SECURE_SCRATCH)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_USE_IMAGE_SIZES_/$(S_INSTALL_USE_IMAGE_SIZES)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_UPGRADE_RETRY_TIMEOUT_/$(S_INSTALL_UPGRADE_RETRY_TIMEOUT)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_UPGRADE_CONNECT_TIMEOUT_/$(S_INSTALL_UPGRADE_CONNECT_TIMEOUT)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH_/$(S_INSTALL_KEYSERVER_RSA_UPGRADE_HASH)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_S_INSTALL_LATENS_UPGRADE_/$(S_INSTALL_LATENS_UPGRADE)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_S_INSTALL_UPGRADE_FORCE_PLATFORM_INIT_/$(S_INSTALL_UPGRADE_FORCE_PLATFORM_INIT)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_S_INSTALL_UPGRADE_WAIT_FOR_NETWORK_/$(S_INSTALL_UPGRADE_WAIT_FOR_NETWORK)/g" $(DESTDIR)/bin/upgrade
	sed -i "s/_S_INSTALL_UPGRADE_WAIT_FOR_UPGRADE_SERVER_/$(S_INSTALL_UPGRADE_WAIT_FOR_UPGRADE_SERVER)/g" $(DESTDIR)/bin/upgrade
	ln -sf ../../../bin/upgrade $(DESTDIR)/usr/local/bin/upgrade

	install -m 0555 upgrade_monitor.sh $(DESTDIR)/etc/init.d/upgrade_monitor

	install -m 0755 $(S_INSTALL_UPGRADE_OK) $(DESTDIR)/etc/init.d/upgrade_ok 
	install -m 0755 upgrade_cli.sh $(DESTDIR)/usr/local/bin/
	if [ -f $(S_INSTALL_UPGRADE_PLATFORM_SPEC)/setnv.sh ]; then \
	    install -m 0755 $(S_INSTALL_UPGRADE_PLATFORM_SPEC)/setnv.sh $(DESTDIR)/usr/local/bin/; \
	else \
	    install -m 0755 ./setnv.sh $(DESTDIR)/usr/local/bin/; \
	fi
	if [ ! -f $(DESTDIR)/usr/sbin/nandcat ]; then \
		echo -e "\x1b[31mError: nandcat tool is required for proper upgrade operation, please pull and rebuild OpenSource/mtd repo\x1b[39m"; \
		exit 1; \
	fi
	make -C $(S_INSTALL_UPGRADE_PLATFORM_SPEC) install
	sed -i "s/\<_UPGRADE_CONFIRMATION_DELAY_\>/$(S_INSTALL_UPGRADE_CONFIRMATION_DELAY)/g" $(DESTDIR)/etc/init.d/upgrade_ok
	#remove comments, white line and indents in upgrade scripts
	sed -e "2,\$$s/#[^\"\'\`]*$$//g" -e "/^\s*$$/d" -e "s/^\s*//" -i \
		$(DESTDIR)/etc/init.d/upgrade_ok \
		$(DESTDIR)/usr/local/bin/upgrade_cli.sh \
		$(DESTDIR)/bin/upgrade \
		$(DESTDIR)/bin/$(SPECFILE) \
		$(DESTDIR)/etc/init.d/upgrade_monitor

clean:
	make -C $(S_INSTALL_UPGRADE_PLATFORM_SPEC) clean

