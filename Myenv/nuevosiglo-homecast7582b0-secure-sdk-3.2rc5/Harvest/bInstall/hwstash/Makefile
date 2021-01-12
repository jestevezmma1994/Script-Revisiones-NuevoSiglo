
all:
ifneq ($(wildcard $(S_INSTALL_BSPEC)/hwstash/*), )
	./prepare_hwstash.sh $(S_INSTALL_BSPEC)/hwstash/ $(ROOTFS)

	for p in cubitv progress QBUpgrade; do \
		cp $(ROOTFS)/usr/local/bin/$${p} $(ROOTFS)/usr/local/bin/$${p}.bin; \
		echo "umount /dev/qbsplash" > $(ROOTFS)/usr/local/bin/$${p}; \
		echo "killall -9 QBCryptoApp" >> $(ROOTFS)/usr/local/bin/$${p}; \
		echo "exec /usr/local/bin/$${p}.bin \$${@}" >> $(ROOTFS)/usr/local/bin/$${p}; \
	done;
endif

