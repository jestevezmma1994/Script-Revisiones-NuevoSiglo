S_INSTALL_FIREWALL?=no
S_INSTALL_NETWORKING_2?=no
S_INSTALL_CABLE_MODEM_SCRIPT?=none
S_INSTALL_CM_IFACE?=eth0
S_INSTALL_NETWORKING_CONFIG_VERSION?=unknown

ifeq (yes,$(S_INSTALL_CONFIG_ENFORCE_HMAC))
ifneq ($(S_INSTALL_KEYSERVER_SWPK5_KEY_ID),)
SWPK5_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SWPK5_KEY_ID))
ifeq ($(SWPK5_KEY),)
$(error Failed to get SWPK5 key)
endif
else
$(error Failed to get SWPK5 key and S_INSTALL_CONFIG_ENFORCE_HMAC=yes)
endif
endif

S_STARTUP_NETWORK_SCRIPT=startup.network
ifeq (exists,$(shell [ -f $(DESTDIR)/usr/sbin/dhclient ] && echo exists))
    S_USE_DHCLIENT=yes
    S_INIT_NETWORK_SCRIPT=init_network_dhclient
    ifeq ($(S_INSTALL_CM_IFACE),none)
        S_CHECK_NETWORKD_SCRIPT=check_networkd_dhclient_no_cable_modem
    else
        S_CHECK_NETWORKD_SCRIPT=check_networkd_dhclient
    endif
else
    S_USE_DHCLIENT=no
    S_INIT_NETWORK_SCRIPT=init_network_udhcpc
    S_CHECK_NETWORKD_SCRIPT=check_networkd_udhcpc
endif

ifeq ($(S_SECURE_BOOT), yes)
ifeq ($(S_SECURE_BOOT_DRYRUN), no)
    S_STARTUP_NETWORK_SCRIPT=startup.network_secure
    ifeq ($(S_USE_DHCLIENT), yes)
        S_INIT_NETWORK_SCRIPT=init_network_dhclient_secure
        ifeq ($(S_INSTALL_CM_IFACE),none)
            S_CHECK_NETWORKD_SCRIPT=check_networkd_dhclient_secure_no_cable_modem
        else
            S_CHECK_NETWORKD_SCRIPT=check_networkd_dhclient_secure
        endif
    else
        S_INIT_NETWORK_SCRIPT=init_network_udhcpc_secure
        S_CHECK_NETWORKD_SCRIPT=check_networkd_udhcpc_secure
    endif
endif
endif


compile:

clean:

install:
#complain if dhclient not found
ifneq ($(S_USE_DHCLIENT),yes)
	$(error ERROR: dhclient missing, exiting.)
endif

	mkdir -p $(DESTDIR)/etc/init.d
	mkdir -p $(DESTDIR)/usr/local/bin
ifeq ($(S_SECURE_BOOT_DRYRUN), yes)
	install -m 0755 startup.inetd $(DESTDIR)/etc/init.d/inetd
endif
	install -m 0755 startup.sockbuf $(DESTDIR)/etc/init.d/sockbuf
ifeq ($(S_INSTALL_NETWORKING_2), no)
	install -m 0755 $(S_STARTUP_NETWORK_SCRIPT) $(DESTDIR)/etc/init.d/network
	install -m 0755 $(S_CHECK_NETWORKD_SCRIPT) $(DESTDIR)/usr/local/bin/check_networkd
	install -m 0755 $(S_INIT_NETWORK_SCRIPT) $(DESTDIR)/usr/local/bin/init_network
	sed -i "s/S_INSTALL_CM_IFACE/$(S_INSTALL_CM_IFACE)/g" $(DESTDIR)/usr/local/bin/check_networkd
	sed -i "s/S_INSTALL_CM_IFACE/$(S_INSTALL_CM_IFACE)/g" $(DESTDIR)/usr/local/bin/init_network
ifeq ($(S_USE_DHCLIENT), yes)
	install -m 0755 dhclient.script $(DESTDIR)/usr/sbin/dhclient-script
else
	mkdir -p $(DESTDIR)/usr/share/udhcpc
	install -m 0755 udhclient.script $(DESTDIR)/usr/share/udhcpc/default.script
endif

endif

	if [ "$(S_INSTALL_BSPEC)" != "" ] && [ -f $(S_INSTALL_BSPEC)/revtunnel/revtunnel.conf ]; then \
		mkdir -p $(DESTDIR)/etc/revtunnel; \
		install -m 0644 $(S_INSTALL_BSPEC)/revtunnel/revtunnel.conf $(DESTDIR)/etc/revtunnel/; \
	fi
	if [ "$(S_INSTALL_BSPEC)" != "" ] && [ -f $(S_INSTALL_BSPEC)/revtunnel/id_rsa.revtunnel ]; then \
		mkdir -p $(DESTDIR)/etc/revtunnel; \
		dropbearconvert openssh dropbear $(S_INSTALL_BSPEC)/revtunnel/id_rsa.revtunnel $(DESTDIR)/etc/revtunnel/id_rsa.revtunnel; \
	fi
ifeq ($(S_INSTALL_FIREWALL),yes)
	if [ "$(S_INSTALL_BSPEC)" != "" ] && [ -f $(S_INSTALL_BSPEC)/firewall.rules ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/firewall.rules $(DESTDIR)/etc/; \
	else \
		install -m 0644 firewall.rules $(DESTDIR)/etc/; \
	fi
endif
ifneq ($(S_INSTALL_CABLE_MODEM_SCRIPT),none)
	install -m 0755 cable-modem/$(S_INSTALL_CABLE_MODEM_SCRIPT) $(DESTDIR)/etc/init.d/cm.startup
endif

ifeq ($(S_INSTALL_NETWORKING_2), yes)
ifneq ($(S_INSTALL_NETWORKING_CONFIG_VERSION), qbconfig)
	install -m 0644 $(S_INSTALL_BSPEC)/network-conf.json $(DESTDIR)/etc/networking/network-conf.json
	install -m 0644 $(S_INSTALL_BSPEC)/operator_interfaces_whitelist.json $(DESTDIR)/etc/networking/operator_interfaces_whitelist.json
	install -m 0644 $(S_INSTALL_BSPEC)/interfaces_types.json $(DESTDIR)/etc/networking/interfaces_types.json
else
	if [ -f $(S_INSTALL_BSPEC)/config/network/const_QBNetworkManagerConf.json ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/config/network/const_QBNetworkManagerConf.json $(DESTDIR)/etc/QBNetworkManagerConf.json; \
	fi
	if [ -f $(S_INSTALL_BSPEC)/config/network/QBNetworkManagerConf.json ]; then \
		install -m 0644 $(S_INSTALL_BSPEC)/config/network/QBNetworkManagerConf.json $(DESTDIR)/etc/networking/QBNetworkManagerConf.json; \
	fi

ifneq ($(SWPK5_KEY),)
	install -d -m 0750 $(DESTDIR)/etc/appkeys
	echo -n $(SWPK5_KEY) | xxd -p -r > $(DESTDIR)/etc/appkeys/swpk5
	chmod o-rwx $(DESTDIR)/etc/appkeys/swpk5
	chmod -w $(DESTDIR)/etc/appkeys/swpk5
	cat $(DESTDIR)/etc/networking/QBNetworkManagerConf.json >/tmp/QBNetworkManagerConf.json.tmp
	echo -n "/etc/vod/networking/QBNetworkManagerConf.json" >> /tmp/QBNetworkManagerConf.json.tmp
	cat /tmp/QBNetworkManagerConf.json.tmp | openssl dgst -sha256 -mac hmac -macopt hexkey:$(SWPK5_KEY) -binary > $(DESTDIR)/etc/networking/QBNetworkManagerConf.json.hmac.1
	cp $(DESTDIR)/etc/networking/QBNetworkManagerConf.json.hmac.1 $(DESTDIR)/etc/networking/QBNetworkManagerConf.json.hmac.2
	rm -rf /tmp/QBNetworkManagerConf.json.tmp
endif
endif
endif
