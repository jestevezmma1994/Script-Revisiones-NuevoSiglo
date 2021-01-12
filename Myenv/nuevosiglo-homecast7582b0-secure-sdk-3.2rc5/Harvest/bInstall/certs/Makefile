# There's a new "cert files" structure that needs somehow to work with new and old apps, and new and old bspecs...
# Because of that we need to maintain both forward and backward compatibility:
# 1) For old bspec we will install old stuff as usual and link new structure into it
# 2) For new bspec we will install new stuff and link old structure into it
#
# New structure is as follows:
# /etc/ssl/transport - same as "public" before, stuff to verify/decrypt partition certs
# /etc/ssl/private - device own certs and keys (rsa, aes)
# /etc/ssl/certs - CA certs, e.g. portal.pem, push.pem, upgrade.pem (and others if needed)
#
# Old structure:
# /etc/certs/public - cacert, certs.key, rsa.pub
# /etc/certs/private - cert.pem, key.pem
#
# VERY IMPORTANT NOTE: pay attention to file permissions.
# Previously permissions for public folder were incorrect! With new approach we're
# fixing this by limiting access to "transport" folder only to root.
#
# We do not want to have ca-path style approach here since we don't want
# an attacker to mimic e.g. push server with mw certficate (if stolen), etc.

S_INSTALL_CERT_LOCATION ?= PART
S_INSTALL_CERT_CRYPTO_VERSION ?= 1
S_MOCKUP_PREFIX ?=

ifeq (2,$(S_INSTALL_CERT_CRYPTO_VERSION))
    S_INSTALL_CERT_SCRIPT=init_certs_2
else
    S_INSTALL_CERT_SCRIPT=init_certs
endif

# Common rule for all targets
COMPAT=common

# Decide which mode to use based on what was said above
ifeq (,$(wildcard $(S_INSTALL_BSPEC)/ssl/*))
	# Old bspec
	COMPAT += compat_old
else
	# New bspec
	COMPAT += compat_new
endif

# Common rule just installs globally used folders (if required)
common:
	# Cleanup
	rm -rf $(SRM_DESTDIR)/etc/ssl
	rm -rf $(SRM_DESTDIR)/etc/certs
ifeq (PART,$(S_INSTALL_CERT_LOCATION))
	install -m 0750 -d $(SRM_DESTDIR)/etc/init.d/
	install -m 0700 $(S_INSTALL_CERT_SCRIPT) $(SRM_DESTDIR)/etc/init.d/init_certs
	sed -i 's#\bS_MOCKUP_PREFIX_STUB\b#$(S_MOCKUP_PREFIX)#g' $(SRM_DESTDIR)/etc/init.d/init_certs
endif

# Reusable rule to create a combined folder structure for old and new methods
form_v2_folders:
	install -m 0750 -d $(SRM_DESTDIR)/etc/ssl
	install -m 0750 -d $(SRM_DESTDIR)/etc/ssl/certs
ifeq (PART,$(S_INSTALL_CERT_LOCATION))
	install -m 0700 -d $(SRM_DESTDIR)/etc/ssl/transport
	install -m 0750 -d $(SRM_DESTDIR)/etc/ssl/backup
else
	install -m 0750 -d $(SRM_DESTDIR)/etc/ssl/private
endif
	# Compatibilty
	install -m 0750 -d $(SRM_DESTDIR)/etc/certs
	install -m 0750 -d $(SRM_DESTDIR)/etc/certs/public

# Install new bspec, link old structure into it
compat_new: form_v2_folders
	@echo "Will install new bSpec and link /etc/certs to /etc/ssl"
	install -m 0640 $(S_INSTALL_BSPEC)/ssl/certs/* $(SRM_DESTDIR)/etc/ssl/certs/
ifeq (PART,$(S_INSTALL_CERT_LOCATION))
# Install Static CA, link private, install transport
	install -m 0600 $(S_INSTALL_BSPEC)/ssl/transport/* $(SRM_DESTDIR)/etc/ssl/transport/
	install -m 0640 $(S_INSTALL_BSPEC)/ssl/backup/* $(SRM_DESTDIR)/etc/ssl/backup
	ln -sf $(S_MOCKUP_PREFIX)/tmp/ssl/private $(SRM_DESTDIR)/etc/ssl/private
	# Compatibility
	ln -sf $(S_MOCKUP_PREFIX)/tmp/ssl/private $(SRM_DESTDIR)/etc/certs/private
	ln -sf $(S_MOCKUP_PREFIX)/etc/ssl/transport/certs.key $(SRM_DESTDIR)/etc/certs/public/certs.key
	ln -sf $(S_MOCKUP_PREFIX)/etc/ssl/transport/rsa.pub $(SRM_DESTDIR)/etc/certs/public/rsa.pub
else
# Install Static CA, install private
	install -m 0640 $(S_INSTALL_BSPEC)/ssl/private/* $(SRM_DESTDIR)/etc/ssl/private
	# Compatibility
	ln -sf $(S_MOCKUP_PREFIX)/etc/ssl/private $(SRM_DESTDIR)/etc/certs/private
endif
	# Compatibility
	ln -sf $(S_MOCKUP_PREFIX)/etc/ssl/certs/cacert.pem $(SRM_DESTDIR)/etc/certs/public/cacert.pem

# Install old bspec, link new structure into it
compat_old: form_v2_folders
	@echo "Will install old bSpec and link /etc/ssl to /etc/certs"
	install -m 0640 $(S_INSTALL_BSPEC)/certs/cacert.pem $(SRM_DESTDIR)/etc/certs/public
ifeq (PART,$(S_INSTALL_CERT_LOCATION))
	install -m 0600 $(S_INSTALL_BSPEC)/certs/certs.key $(SRM_DESTDIR)/etc/certs/public/certs.key
	install -m 0600 $(S_INSTALL_BSPEC)/certs/rsa.pub $(SRM_DESTDIR)/etc/certs/public/rsa.pub
	ln -sf $(S_MOCKUP_PREFIX)/tmp/ssl/private $(SRM_DESTDIR)/etc/certs/private
	install -m 0640 $(S_INSTALL_BSPEC)/keys/upgrade-https/cert.pem $(SRM_DESTDIR)/etc/ssl/backup/cert.pem
	install -m 0640 $(S_INSTALL_BSPEC)/keys/upgrade-https/certkey.pem $(SRM_DESTDIR)/etc/ssl/backup/key.pem
	# Compatibility
	ln -sf $(S_MOCKUP_PREFIX)/etc/certs/public/certs.key $(SRM_DESTDIR)/etc/ssl/transport/certs.key
	ln -sf $(S_MOCKUP_PREFIX)/etc/certs/public/rsa.pub $(SRM_DESTDIR)/etc/ssl/transport/rsa.pub
	ln -sf $(S_MOCKUP_PREFIX)/tmp/ssl/private $(SRM_DESTDIR)/etc/ssl/private
else
	install -m 0750 -d $(SRM_DESTDIR)/etc/certs/private
	install -m 0640 $(S_INSTALL_BSPEC)/cert-files/* $(SRM_DESTDIR)/etc/certs/private
	# Compatibility
	rm -rf $(SRM_DESTDIR)/etc/ssl/private
	ln -sf $(S_MOCKUP_PREFIX)/etc/certs/private $(SRM_DESTDIR)/etc/ssl/private
endif
	# Compatibility
	ln -sf $(S_MOCKUP_PREFIX)/etc/certs/public/cacert.pem $(SRM_DESTDIR)/etc/ssl/certs/cacert.pem
	ln -sf $(S_MOCKUP_PREFIX)/etc/certs/public/cacert.pem $(SRM_DESTDIR)/etc/ssl/certs/device.pem

install: $(COMPAT)

clean:

