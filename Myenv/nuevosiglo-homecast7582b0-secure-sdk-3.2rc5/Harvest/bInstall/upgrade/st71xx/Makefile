ifeq (,${DESTDIR})
  $(error DESTDIR is not defined)
endif
ifeq (,${SPECFILE})
  $(error SPECFILE is not defined)
endif

S_INSTALL_UPGRADE_USR_BASE_PART_NO?=5

compile: $(TOOLSDIR)/nvimage

clean:

$(TOOLSDIR)/nvimage: nvimage.c
	srm-local-run gcc -o $@ nvimage.c || srm-local-run gcc -o $@ -m32 nvimage.c

install:
	install -m 0755 ${S_INSTALL_UPGRADE_PLATFORM_PROC} ${DESTDIR}/bin/${SPECFILE}
	sed -e "s/_UPGRADE_USR_BASE_PART_NO_/$(S_INSTALL_UPGRADE_USR_BASE_PART_NO)/g" \
		-e "s/_UPGRADE_FILES_/$(UPGRADE_FILES)/g" \
		-i ${DESTDIR}/bin/${SPECFILE}
	srm-local-run install -s -m 0755 $(TOOLSDIR)/nvimage $(BUILD_ROOT)/usr/bin/
	install -m 0755 mkfactoryimage-opentech.sh $(DESTDIR)/../../
	sed -e "s/S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID/$(S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID)/g" \
		-i $(DESTDIR)/../../mkfactoryimage-opentech.sh
	sed -e "s/S_INSTALL_KEYSERVER_RSA_BOOT_HASH/$(S_INSTALL_KEYSERVER_RSA_BOOT_HASH)/g" \
		-i $(DESTDIR)/../../mkfactoryimage-opentech.sh
	install -m 0755 mkfactoryimage-intek.sh $(DESTDIR)/../../
	sed -e "s/S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID/$(S_INSTALL_KEYSERVER_RSA_BOOT_KEY_ID)/g" \
		-i $(DESTDIR)/../../mkfactoryimage-intek.sh
	sed -e "s/S_INSTALL_KEYSERVER_RSA_BOOT_HASH/$(S_INSTALL_KEYSERVER_RSA_BOOT_HASH)/g" \
		-i $(DESTDIR)/../../mkfactoryimage-intek.sh
