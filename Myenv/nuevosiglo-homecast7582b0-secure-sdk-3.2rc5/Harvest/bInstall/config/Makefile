# Configuration
# 	S_INSTALL_CONFIG_SH - is a name of script file in ./config.sh/ to be used in init.d
#
##########################################
#       DEFAULT CONFIGURATION ############
S_INSTALL_CONFIG_SH ?= generic
S_INSTALL_CLI_PROMPT ?= $(SRM_PROJECT)
S_INSTALL_CONFIG_PARTITION ?= dynamic
S_INSTALL_CONFIG_FORCE_RESET ?= no
S_INSTALL_CONFIG_FORCE_RESET_SCRIPT ?= generic
S_INSTALL_CONFIG_USE_JSON ?= no
S_INSTALL_CONFIG_ENFORCE_HMAC ?= no

ifeq (,$(S_INSTALL_BSPEC))
$(warning bspec.comp component is missing in the build!);
endif

MAP=$(S_MAP_DIR)/all.maps

ifeq (yes,$(S_INSTALL_CONFIG_ENFORCE_HMAC))
ifneq (,$(S_INSTALL_KEYSERVER_SWPK5_KEY_ID))
SWPK5_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SWPK5_KEY_ID))
ifeq (,$(SWPK5_KEY))
$(error Failed to get SWPK5 key)
endif
else
$(error Failed to get SWPK5 key and S_INSTALL_ENFORCE_HMAC_QBCONFIG=yes)
endif
endif

all: install

install:
	mkdir -p $(DESTDIR)/etc
ifeq (no, $(S_INSTALL_CONFIG_USE_JSON))
	mkdir -p $(DESTDIR)/etc/conf
endif
	mkdir -p $(DESTDIR)/etc/default-conf
	mkdir -p $(DESTDIR)/usr/local/bin
	#firmware identification files
	echo -n $(SRM_BOARD) > $(DESTDIR)/etc/BOARD
	echo -n $(SRM_PROJECT) > $(DESTDIR)/etc/BUILD
	#set needed for upgrade
	echo -n $(S_INSTALL_UPGRADE_ARCH) > $(DESTDIR)/etc/ARCH
	echo -n $(S_INSTALL_UPGRADE_IMG) > $(DESTDIR)/etc/IMG
	echo -n $(S_INSTALL_UPGRADE_PROJECT) > $(DESTDIR)/etc/PROJECT
	#string for upgrade and disk formating
	for i in format strings upgrade; do cp strings/$$i.$(S_INSTALL_LANG) $(DESTDIR)/etc/$$i.txt; done
	#base package identification
	mkdir -p $(DESTDIR)/etc/base/
	echo -n $(SRM_RELEASE)rc$(SRM_RELEASE_CANDIDATE) > $(DESTDIR)/etc/base/$(SRM_PROJECT)
	#install initial config settings
	if [ -f $(S_INSTALL_BSPEC)/config/volume ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/config/volume $(DESTDIR)/etc/default-conf/volume; \
	fi
ifeq (no, $(S_INSTALL_CONFIG_USE_JSON))
	install -m 0644 $(S_INSTALL_BSPEC)/config/conf.xml $(DESTDIR)/etc/default-conf/conf.xml
	ln -sfn conf.xml $(DESTDIR)/etc/default-conf/conf.link
else
	install -m 0644 $(S_INSTALL_BSPEC)/config/conf.json $(DESTDIR)/etc/default-conf/conf.json
	install -m 0644 $(S_INSTALL_BSPEC)/config/const_conf.json $(DESTDIR)/etc/conf.json
	# test json config by running it through JSON parser
	qbconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.json
	qbconf_ctl.py -n -i $(DESTDIR)/etc/conf.json
	# Sanity check: const vs. default json config
	qbconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.json -c $(DESTDIR)/etc/conf.json
	ln -sfn conf.json $(DESTDIR)/etc/default-conf/conf.link
endif
	if [ "$(S_INSTALL_CONFIG_USE_JSON)" = "no" ]; then \
		rm $(DESTDIR)/etc/STB.cfg; \
		svconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.xml >$(DESTDIR)/etc/STB.cfg; \
		svconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.xml CONFIG_VERSION $(SRM_RELEASE); \
		svconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.xml ORIGINAL_CONFIG_VERSION $(SRM_RELEASE); \
	else \
		if [ "$(S_SECURE_BOOT)" = "yes" -a "$(S_SECURE_BOOT_DRYRUN)" = "no" ]; then \
			qbconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.json >$(DESTDIR)/etc/STB.cfg; \
		else \
			ln -sfn /tmp/STB.cfg $(DESTDIR)/etc/STB.cfg; \
		fi; \
		qbconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.json CONFIG_VERSION $(SRM_RELEASE); \
		qbconf_ctl.py -n -i $(DESTDIR)/etc/default-conf/conf.json ORIGINAL_CONFIG_VERSION $(SRM_RELEASE); \
	fi \

ifneq (,$(SWPK5_KEY))
	install -d -m 0750 $(DESTDIR)/etc/appkeys
	echo -n $(SWPK5_KEY) | xxd -p -r > $(DESTDIR)/etc/appkeys/swpk5
	chmod o-rwx $(DESTDIR)/etc/appkeys/swpk5
	chmod -w $(DESTDIR)/etc/appkeys/swpk5
	cat $(DESTDIR)/etc/default-conf/conf.json >/tmp/conf.json.tmp
	echo -n "/etc/vod/conf.json" >> /tmp/conf.json.tmp
	cat /tmp/conf.json.tmp | openssl dgst -sha256 -mac hmac -macopt hexkey:$(SWPK5_KEY) -binary > $(DESTDIR)/etc/default-conf/conf.json.hmac.1
	cp $(DESTDIR)/etc/default-conf/conf.json.hmac.1 $(DESTDIR)/etc/default-conf/conf.json.hmac.2
	rm -rf /tmp/conf.json.tmp
endif
	#config specifications
	if [ -f $(S_INSTALL_BSPEC)/upgradeInfo ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/upgradeInfo $(DESTDIR)/etc/upgradeInfo; \
	fi
	if [ -f $(S_INSTALL_BSPEC)/upgradeDVBInfo ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/upgradeDVBInfo $(DESTDIR)/etc/upgradeDVBInfo; \
	fi
ifeq (no, $(S_INSTALL_CONFIG_USE_JSON))
	install -m 0644 $(S_INSTALL_BSPEC)/config/spec/*.xml $(DESTDIR)/etc/conf
	sed -i -e 's#S_INSTALL_CLI_PROMPT#$(S_INSTALL_CLI_PROMPT) #' $(DESTDIR)/etc/conf/cli_spec.xml
	sed -i -e 's#SRM_CLI_PROMPT#$(S_INSTALL_CLI_PROMPT) #' $(DESTDIR)/etc/conf/cli_spec.xml
endif
	#system init.d scripts
	mkdir -p $(DESTDIR)/etc/init.d
	install -m 0755 config.sh/$(S_INSTALL_CONFIG_SH) $(DESTDIR)/etc/init.d/config
	sed -i 's#S_INSTALL_CONFIG_PARTITION#$(S_INSTALL_CONFIG_PARTITION)#g' $(DESTDIR)/etc/init.d/config
	#upgrade.d scripts
	mkdir -p $(DESTDIR)/etc/upgrade.d/
	install -m 0755 upgrade.default-conf $(DESTDIR)/etc/upgrade.d/
	#GUI gui.d scripts
	mkdir -p $(DESTDIR)/etc/gui.d
	install -m 0755 none.sh $(DESTDIR)/etc/gui.d/none
	install -m 0755 mpmode.sh $(DESTDIR)/etc/gui.d/mpmode
	[ -f $(DESTDIR)/etc/gui.d/default ] || ln -sf none $(DESTDIR)/etc/gui.d/default
	#user config scritps
	mkdir -p $(DESTDIR)/usr/local/bin
	install -m 0755 store_config.sh load_config.sh $(DESTDIR)/usr/local/bin
	#whitelist for shell commands
	if [ -f $(S_INSTALL_BSPEC)/QBShellCmdsWhitelist ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/QBShellCmdsWhitelist $(DESTDIR)/etc/QBShellCmdsWhitelist; \
		fi
#  Installing /etc/configreset marker to force reset on first boot after upgrade
ifeq (yes,$(S_INSTALL_CONFIG_FORCE_RESET))
	touch $(DESTDIR)/etc/configreset
	chmod 0755 $(DESTDIR)/etc/configreset
	install -m 0755 configreset/$(S_INSTALL_CONFIG_FORCE_RESET_SCRIPT) $(DESTDIR)/usr/local/bin/resetaction.sh
endif
#	echo $(SRM_RELEASE) > $(SRM_DESTDIR)/etc/RELEASE
#	echo $(SRM_RELEASE) | cut -d '.' -f 1-2 > $(SRM_DESTDIR)/etc/VERSION
	if [ ! -z "$(shell grep vmx_data0 $(MAP))" ]; then \
		if [ ! -z "$(shell grep device-type=block $(MAP))" ]; then \
			echo "Installing VMX mounting scripts for MMC"; \
			install -m 0755 mount_vmx_data-mmc.sh $(DESTDIR)/etc/init.d/mount_vmx_data; \
		else \
			echo "Installing VMX mounting scripts for NAND"; \
			install -m 0755 mount_vmx_data-nand.sh $(DESTDIR)/etc/init.d/mount_vmx_data; \
		fi; \
		install -d -m 0755 $(DESTDIR)/etc/vmx0; \
		install -d -m 0755 $(DESTDIR)/etc/vmx1; \
	fi
	if [ -f $(S_INSTALL_BSPEC)/config/viewRightManager ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/config/viewRightManager $(DESTDIR)/etc/viewRightManager; \
	fi

clean:
	@echo nothing to do for clean
