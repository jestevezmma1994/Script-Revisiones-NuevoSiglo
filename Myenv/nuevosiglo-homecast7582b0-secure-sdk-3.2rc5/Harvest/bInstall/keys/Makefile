# install keys used for encryption
#
# below moved to main makefile
#S_INSTALL_SSHKEYS_DIR?=generic
#S_INSTALL_UPCCERT_DIR?=generic
#S_INSTALL_UPPCERT_DIR?=generic

ifneq (,$(S_INSTALL_SSLCERT_DIR))
ifeq (,$(S_INSTALL_UPCCERT_DIR))
$(error variable S_INSTALL_SSLCERT_DIR is deprecated, please use S_INSTALL_UPCCERT_DIR instead (find it in *.prj)
endif
endif

S_INSTALL_SSHKEYS?=yes

ifneq (,$(S_INSTALL_KEYSERVER_SWPK4_KEY_ID))
SWPK4_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SWPK4_KEY_ID))
ifeq (,$(SWPK4_KEY))
$(error Failed to get SWPK4 key)
endif
endif

ifneq (,$(S_INSTALL_KEYSERVER_SANDBOX_ENC_ID))
SNDB_ENC_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SANDBOX_ENC_ID))
ifeq (,$(SNDB_ENC_KEY))
$(error Failed to get sandbox ENC key)
endif
endif

S_INSTALL_ENCRYPT_NVRAM ?= no
S_INSTALL_NVRAM_KEY_LOCATION ?= ROOTFS

S_INSTALL_VERIFY_NVRAM ?= no
USE_QB_IMAGE=$(shell mapreader.pl $(S_MAP_DIR)/all.maps attr name=nvram | grep -o "qbimage")
ifneq (,$(USE_QB_IMAGE))
    S_INSTALL_VERIFY_NVRAM := yes
    S_INSTALL_ENCRYPT_NVRAM := yes
endif

ifeq (ROOTFS,$(S_INSTALL_NVRAM_KEY_LOCATION))
    ifeq (yes,$(S_INSTALL_ENCRYPT_NVRAM))
        NVRAM_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SYM_NVRAM_KEY_ID))
        ifeq (,$(NVRAM_KEY))
            $(error Failed to get NVRAM key)
        endif
    endif
endif

ifeq (yes,$(S_INSTALL_VERIFY_NVRAM))
NVRAM_HMAC_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SYM_NVRAM_HMAC_KEY_ID))
ifeq (,$(NVRAM_HMAC_KEY))
$(error Failed to get NVRAM HMAC key)
endif
endif

S_INSTALL_ENCRYPT_PVR_META ?= no
ifeq (yes,$(S_INSTALL_ENCRYPT_PVR_META))
META_KEY := $(shell srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SYM_META_KEY_ID))
ifeq (,$(META_KEY))
$(error Failed to get META key)
endif
endif

# Decide which mode for bspec certs to use: either old or a new structure?
ifeq (,$(wildcard $(S_INSTALL_BSPEC)/ssl/*))
	# Old bspec
	COMPAT = compat_old
else
	# New bspec
	COMPAT = compat_new
endif

compile:
	echo "Compile not implemented"

install: .install $(COMPAT)

compat_new:
	# Instead of the actual certificate from bSpec just link to a common device cert
	# from the proper location
	rm -f $(SRM_DESTDIR)/etc/cacert.pem
	rm -f $(SRM_DESTDIR)/etc/cert.pem
	rm -f $(SRM_DESTDIR)/etc/certkey.pem
	ln -sf /etc/ssl/certs/cacert.pem $(SRM_DESTDIR)/etc/cacert.pem
	ln -sf /etc/ssl/private/cert.pem $(SRM_DESTDIR)/etc/cert.pem
	ln -sf /etc/ssl/private/key.pem $(SRM_DESTDIR)/etc/certkey.pem

compat_old:
	# Generic/legacy certificates (moved here from .install target)
	# Upgrade server certificates (server cert and client key)
	rm -f $(SRM_DESTDIR)/etc/cacert.pem
	rm -f $(SRM_DESTDIR)/etc/cert.pem
	rm -f $(SRM_DESTDIR)/etc/certkey.pem
	install -m 0644 $(S_INSTALL_BSPEC)/keys/upgrade-https/cacert.pem $(SRM_DESTDIR)/etc/cacert.pem
	install -m 0644 $(S_INSTALL_BSPEC)/keys/upgrade-https/cert.pem $(SRM_DESTDIR)/etc/cert.pem
	install -m 0644 $(S_INSTALL_BSPEC)/keys/upgrade-https/certkey.pem $(SRM_DESTDIR)/etc/certkey.pem
	# Upgrade package (ECDSA and RSA2048)
ifeq (yes,$(S_INSTALL_LEGACY_ECDSA_UPGRADE))
	install -m 0644 $(S_INSTALL_BSPEC)/keys/upgrade-package/ca.cert.der $(SRM_DESTDIR)/etc/up.cacert.der
endif

.install:
	#VM Rootcert keys
ifeq (yes,$(S_INSTALL_VMROOTCERT))
	mkdir -p $(SRM_DESTDIR)/etc/cert
	install -m 0644 $(S_INSTALL_BSPEC)/keys/vm/rootcert.pem $(SRM_DESTDIR)/etc/cert
endif
	#SSH keys
ifeq (yes,$(S_INSTALL_SSHKEYS))
	install -m 0600 $(S_INSTALL_BSPEC)/keys/ssh/id_rsa.pub $(SRM_DESTDIR)/etc/authorized_keys
	install -m 0600 $(S_INSTALL_BSPEC)/keys/ssh/id_rsa $(SRM_DESTDIR)/../../id_rsa
endif
ifeq (yes,$(S_INSTALL_USE_RSA_UPGRADE))
ifeq (yes,$(S_INSTALL_USE_KEYSERVER))
	srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_RSA_UPGRADE_INNER_KEY_ID) > $(SRM_DESTDIR)/etc/id_rsa.package.pub
else
	install -m 0644 $(S_INSTALL_BSPEC)/keys/upgrade-package/id_rsa.package.pub $(SRM_DESTDIR)/etc/id_rsa.package.pub
endif
endif

ifeq (yes,$(S_INSTALL_USE_KEYSERVER))
	srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_RSA_SKIN_KEY_ID) > $(SRM_DESTDIR)/etc/id_rsa.skin.pub
endif

	mkdir -p $(SRM_DESTDIR)/etc/keys/
ifneq (,$(S_INSTALL_KEYSERVER_SWPK4_KEY_ID))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
	echo $(SWPK4_KEY) | xxd -r -p - > $(SRM_DESTDIR)/etc/keys/swpk4
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/swpk4
endif
ifneq (,$(S_INSTALL_KEYSERVER_SANDBOX_ENC_ID))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
	echo $(SNDB_ENC_KEY) | xxd -r -p - > $(SRM_DESTDIR)/etc/keys/sandbox_enc
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/sandbox_enc
endif
ifneq (,$(S_INSTALL_KEYSERVER_SANDBOX_RSA_ID))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
	srm-host-run jclient retrieve $(S_INSTALL_KEYSERVER_SANDBOX_RSA_ID) > $(SRM_DESTDIR)/etc/keys/sandbox_rsa
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/sandbox_rsa
endif
ifeq (yes,$(S_INSTALL_ENCRYPT_NVRAM))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
    ifeq (EXTERNAL,$(S_INSTALL_NVRAM_KEY_LOCATION))
	ln -sf /tmp/keys/nvram $(SRM_DESTDIR)/etc/keys/nvram
    else
	echo $(NVRAM_KEY) | xxd -r -p - $(SRM_DESTDIR)/etc/keys/nvram
    endif
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/nvram
endif
ifeq (yes,$(S_INSTALL_VERIFY_NVRAM))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
	echo $(NVRAM_HMAC_KEY) | xxd -r -p - $(SRM_DESTDIR)/etc/keys/nvram_hmac
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/nvram_hmac
endif
ifeq (yes,$(S_INSTALL_ENCRYPT_PVR_META))
	install -m 0755 -d $(SRM_DESTDIR)/etc/keys
	echo $(META_KEY) >$(SRM_DESTDIR)/etc/keys/meta
	chmod go-rwx $(SRM_DESTDIR)/etc/keys/meta
endif
	tar zcvf $(BUILDDIR)/key-tools.tgz key-tools
ifneq (,$(wildcard ./stb-tools/$(S_INSTALL_UPGRADE_PLATFORM_SPEC)))
	$(MAKE) -C ./stb-tools/$(S_INSTALL_UPGRADE_PLATFORM_SPEC)
endif

clean:
