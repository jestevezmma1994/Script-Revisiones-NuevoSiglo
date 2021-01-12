ifeq (,${DESTDIR})
  $(error DESTDIR is not defined)
endif
ifeq (,${SPECFILE})
  $(error SPECFILE is not defined)
endif

S_INSTALL_UPGRADE_SLOT0_KERN_PART_ID ?= CS2-Part4
S_INSTALL_UPGRADE_SLOT0_ROOT_PART_ID ?= CS2-Part6
S_INSTALL_UPGRADE_SLOT1_KERN_PART_ID ?= CS2-Part5
S_INSTALL_UPGRADE_SLOT1_ROOT_PART_ID ?= CS2-Part7

install:
	install -m 0755 ${S_INSTALL_UPGRADE_PLATFORM_PROC} ${DESTDIR}/bin/${SPECFILE}
	sed -e "2,\$$s/#[^\"\'\`]*$$//g" -e "/^\s*$$/d" -e "s/^\s*//" -i ${DESTDIR}/bin/${SPECFILE}
	sed  -i "s#S_INSTALL_UPGRADE_SLOT0_KERN_PART_ID#$(S_INSTALL_UPGRADE_SLOT0_KERN_PART_ID)#g" ${DESTDIR}/bin/${SPECFILE}
	sed  -i "s#S_INSTALL_UPGRADE_SLOT0_ROOT_PART_ID#$(S_INSTALL_UPGRADE_SLOT0_ROOT_PART_ID)#g" ${DESTDIR}/bin/${SPECFILE}
	sed  -i "s#S_INSTALL_UPGRADE_SLOT1_KERN_PART_ID#$(S_INSTALL_UPGRADE_SLOT1_KERN_PART_ID)#g" ${DESTDIR}/bin/${SPECFILE}
	sed  -i "s#S_INSTALL_UPGRADE_SLOT1_ROOT_PART_ID#$(S_INSTALL_UPGRADE_SLOT1_ROOT_PART_ID)#g" ${DESTDIR}/bin/${SPECFILE}
	[ ! -f $(SRM_BOARD)_blink.sh ] || install -m 0755 $(SRM_BOARD)_blink.sh ${DESTDIR}/usr/local/bin/blink
	for i in boot_config_tools.sh confirm_upgrade.sh boot_slot.sh; do \
		install -m 0755 $$i ${DESTDIR}/bin/ && ln -sf ../../../bin/$$i  ${DESTDIR}/usr/local/bin/$$i; \
		sed -e "2,\$$s/#[^\"\'\`]*$$//g" -e "/^\s*$$/d" -e "s/^\s*//" -i ${DESTDIR}/bin/$$i; \
	done;
		

clean:
	echo nothing to do
